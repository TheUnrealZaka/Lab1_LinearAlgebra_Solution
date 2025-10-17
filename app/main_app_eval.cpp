#include <SDL3/SDL.h>
#include <GL/glew.h>
#include <cstdio>
#include <string>
#include <random>
#include <cmath>

// Dear ImGui
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"

// Project headers
#include "Matrix.hpp"
#include "LinAlg.hpp"
#include "Solve.hpp"
#include "DatasetIO.hpp"
#include "OpsCounter.hpp"
#include "Timer.hpp"

static void Check(bool ok, const char* msg) {
    if (!ok) { std::fprintf(stderr, "%s: %s", msg, SDL_GetError()); std::fflush(stderr); std::exit(1); }
}

// ---------------- UI State & helpers ----------------
// Panells
static bool show_datasets = true;
static bool show_ops = true;
static bool show_results = true;

// Dimensions (permet no-quadrat per a MatVec/MatMul)
static int m_rows = 700; // files d'A
static int a_cols = 700; // cols d'A
static int b_rows = 700; // files de B
static int n_cols = 700; // cols de B
static int x_len = 700;  // |x|
static int p_cols = 700; // (MatMul3) cols de B = files de C
static int q_cols = 700; // (MatMul3) cols de C

// Solve sempre quadrada
static int n_solve = 700;

// Paràmetres
static double tol = 1e-12;
static int op_selected = 0; // 0=MatVec, 1=MatMul, 2=Solve(NoPivot), 3=Solve(PartialPivot), 4=MatMul3, 5=Solve(Pivot Aug)

// Dades
static Matrix A, B, C3; // entrades
static Vec    x, b;     // x i b

// Resultats
static Vec    y;      // per MatVec
static Matrix C;      // per MatMul/MatMul3
static Vec    x_sol;  // per Solve

enum class SolveStatus { Ok, Singular, PivotFailure, DimError, None };
static SolveStatus last_status = SolveStatus::None;

// Comptatge i temps
static OpsCounter last_ops{};
static double     last_time_ms = 0.0;
static double     last_rel_res = std::numeric_limits<double>::quiet_NaN();

// Forward dels mètodes nous
Matrix Multiply3(const Matrix& A, const Matrix& B, const Matrix& C, OpsCounter* op = nullptr);
namespace LinAlg { SolveReport SolvePivotAugmented(Matrix Ab, double tol); }

// Helpers UI: renderitzar taules breus (n <= 8 o preview 8x8)
static void DrawVecPreview(const char* label, Vec& v, bool editable)
{
    ImGui::SeparatorText(label);
    int m = (int)std::min<std::size_t>(8, v.size());
    if (m == 0) { ImGui::TextDisabled("(buit)"); return; }
    if (ImGui::BeginTable(label, m, ImGuiTableFlags_Borders)) {
        ImGui::TableNextRow();
        for (int j = 0; j < m; ++j) {
            ImGui::TableSetColumnIndex(j);
            if (editable) {
                double tmp = v[(size_t)j];
                ImGui::SetNextItemWidth(80);
                if (ImGui::InputDouble((std::string(label) + "[" + std::to_string(j) + "]").c_str(), &tmp, 0, 0, "%.4f")) {
                    v[(size_t)j] = tmp;
                }
            }
            else {
                ImGui::Text("%.4f", v[(size_t)j]);
            }
        }
        ImGui::EndTable();
    }
    if ((int)v.size() > m) ImGui::TextDisabled("... (%d total)", (int)v.size());
}

static void DrawMatPreview(const char* label, Matrix& M, bool editable)
{
    ImGui::SeparatorText(label);
    if (M.rows == 0 || M.cols == 0) { ImGui::TextDisabled("(buida)"); return; }
    int r = (int)std::min<std::size_t>(8, M.rows);
    int c = (int)std::min<std::size_t>(8, M.cols);
    if (ImGui::BeginTable(label, c, ImGuiTableFlags_Borders)) {
        for (int i = 0; i < r; ++i) {
            ImGui::TableNextRow();
            for (int j = 0; j < c; ++j) {
                ImGui::TableSetColumnIndex(j);
                if (editable) {
                    double tmp = M.At((size_t)i, (size_t)j);
                    ImGui::SetNextItemWidth(80);
                    if (ImGui::InputDouble((std::string(label) + "[" + std::to_string(i) + "," + std::to_string(j) + "]").c_str(), &tmp, 0, 0, "%.4f")) {
                        M.At((size_t)i, (size_t)j) = tmp;
                    }
                }
                else {
                    ImGui::Text("%.4f", M.At((size_t)i, (size_t)j));
                }
            }
        }
        ImGui::EndTable();
    }
    if ((int)M.rows > r || (int)M.cols > c) ImGui::TextDisabled("... (preview %dx%d de %zux%zu)", r, c, M.rows, M.cols);
}

int main()
{
    // Logs SDL
    SDL_SetLogPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_VERBOSE);

    // 1) SDL init
    Check(SDL_Init(SDL_INIT_VIDEO), "SDL_Init failed");

    // 2) Atributs GL + finestra
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_Window* window = SDL_CreateWindow(
        "Laboratori 1 (Eval): Algebra Lineal Numerica",
        1280, 720,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );
    Check(window != nullptr, "SDL_CreateWindow failed");

    SDL_GLContext gl_ctx = SDL_GL_CreateContext(window);
    Check(gl_ctx != nullptr, "SDL_GL_CreateContext failed");
    SDL_GL_MakeCurrent(window, gl_ctx);
    SDL_GL_SetSwapInterval(1); // vsync

    // 3) GLEW (després de tenir context)
    glewExperimental = GL_TRUE;
    GLenum glew_err = glewInit();
    if (glew_err != GLEW_OK)
    {
        std::fprintf(stderr, "glewInit failed"); return 1;
    }

    // 4) Dear ImGui: context + estil + backends
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    // Backends: SDL3 + OpenGL3
    ImGui_ImplSDL3_InitForOpenGL(window, gl_ctx);
    const char* glsl_version = "#version 330"; // GLSL version string per a backend OpenGL3
    ImGui_ImplOpenGL3_Init(glsl_version);

    // ---------------- Bucle principal ----------------
    bool running = true;
    while (running)
    {
        // --- EVENTS ---
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            ImGui_ImplSDL3_ProcessEvent(&e);
            if (e.type == SDL_EVENT_QUIT) running = false;
            if (e.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
                e.window.windowID == SDL_GetWindowID(window)) running = false;
        }

        // --- NEW FRAME ---
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        // --- PANELL: DATASETS ---
        if (show_datasets)
        {
            if (ImGui::Begin("Datasets", &show_datasets))
            {
                ImGui::TextWrapped("Carrega o genera conjunts de prova.");
                ImGui::Separator();

                // Dimensions segons operació
                if (op_selected == 0) { // MatVec
                    ImGui::InputInt("m (files A)", &m_rows); if (m_rows < 1) m_rows = 1;
                    ImGui::InputInt("nA (cols A = |x|)", &a_cols); if (a_cols < 1) a_cols = 1;
                    ImGui::BeginDisabled(true);
                    ImGui::InputInt("kB (files B)", &b_rows);
                    ImGui::InputInt("n (cols B)", &n_cols);
                    ImGui::EndDisabled();
                    x_len = a_cols;
                }
                else if (op_selected == 1) { // MatMul
                    ImGui::InputInt("m (files A)", &m_rows); if (m_rows < 1) m_rows = 1;
                    ImGui::InputInt("nA (cols A = files B)", &a_cols); if (a_cols < 1) a_cols = 1;
                    ImGui::InputInt("nB (cols B)", &n_cols); if (n_cols < 1) n_cols = 1;
                    b_rows = a_cols;
                }
                else if (op_selected == 4) { // MatMul3
                    ImGui::InputInt("m (files A)", &m_rows);        if (m_rows < 1) m_rows = 1;
                    ImGui::InputInt("nA (cols A = files B)", &a_cols); if (a_cols < 1) a_cols = 1;
                    ImGui::InputInt("p (cols B = files C)", &p_cols);  if (p_cols < 1) p_cols = 1;
                    ImGui::InputInt("q (cols C)", &q_cols);          if (q_cols < 1) q_cols = 1;
                    b_rows = a_cols; // B.rows
                }
                else { // Solve
                    ImGui::InputInt("n (mida quadrada)", &n_solve); if (n_solve < 1) n_solve = 1;
                }

                if (ImGui::Button("Genera aleatori"))
                {
                    // RNG amb llavor variable per evitar repetir
                    std::random_device rd; std::mt19937 rng(rd());
                    std::uniform_real_distribution<double> U(-1.0, 1.0);

                    if (op_selected == 0) { // MatVec
                        int m = m_rows, k = a_cols;
                        A = Matrix(m, k, 0.0);
                        x = Vec((size_t)k);
                        for (int i = 0; i < m; ++i) {
                            for (int j = 0; j < k; ++j) { A.At((size_t)i, (size_t)j) = U(rng) * 0.5; }
                            if (i < k) A.At((size_t)i, (size_t)i) += 1.0;
                        }
                        for (int j = 0; j < k; ++j) x[(size_t)j] = U(rng);
                        B = Matrix(); C3 = Matrix();
                    }
                    else if (op_selected == 1) { // MatMul
                        int m = m_rows, k = a_cols, n = n_cols;
                        A = Matrix(m, k, 0.0);
                        B = Matrix(k, n, 0.0);
                        for (int i = 0; i < m; ++i)
                            for (int j = 0; j < k; ++j) A.At((size_t)i, (size_t)j) = U(rng);
                        for (int i = 0; i < k; ++i)
                            for (int j = 0; j < n; ++j) B.At((size_t)i, (size_t)j) = U(rng);
                        x.clear(); C3 = Matrix();
                    }
                    else if (op_selected == 4) { // MatMul3
                        int m = m_rows, nA = a_cols, p = p_cols, q = q_cols;
                        A = Matrix(m, nA, 0.0);
                        B = Matrix(nA, p, 0.0);
                        C3 = Matrix(p, q, 0.0);
                        for (int i = 0; i < m; ++i)
                            for (int j = 0; j < nA; ++j) A.At((size_t)i, (size_t)j) = U(rng);
                        for (int i = 0; i < nA; ++i)
                            for (int j = 0; j < p; ++j) B.At((size_t)i, (size_t)j) = U(rng);
                        for (int i = 0; i < p; ++i)
                            for (int j = 0; j < q; ++j) C3.At((size_t)i, (size_t)j) = U(rng);
                        x.clear();
                    }
                    else { // Solve
                        int n = n_solve;
                        A = Matrix(n, n, 0.0); b = Vec((size_t)n);
                        for (int i = 0; i < n; ++i) {
                            double rowsum = 0.0;
                            for (int j = 0; j < n; ++j)
                                if (i != j) { double val = U(rng); A.At((size_t)i, (size_t)j) = val; rowsum += std::fabs(val); }
                            A.At((size_t)i, (size_t)i) = rowsum + 1.0; // diag. dominant
                            b[(size_t)i] = U(rng);
                        }
                        B = Matrix(); C3 = Matrix(); x.clear();
                    }
                    // neteja resultats
                    y.clear(); C = Matrix(); x_sol.clear();
                    last_ops = OpsCounter{}; last_time_ms = 0.0; last_rel_res = std::numeric_limits<double>::quiet_NaN(); last_status = SolveStatus::None;
                }

                ImGui::Separator();
                ImGui::Text("Tolerancia numerica (pivot ~ 0)");
                ImGui::InputDouble("tol", &tol, 0, 0, "%.1e");

                bool small = false;
                if (op_selected == 0)
                    small = (A.rows <= 8 && A.cols <= 8);
                else if (op_selected == 1)
                    small = (A.rows <= 8 && A.cols <= 8) && (B.rows <= 8 && B.cols <= 8);
                else if (op_selected == 4)
                    small = (A.rows <= 8 && A.cols <= 8) && (B.rows <= 8 && B.cols <= 8) && (C3.rows <= 8 && C3.cols <= 8);
                else
                    small = (A.rows <= 8 && A.cols <= 8) && (b.size() <= 8);

                DrawMatPreview("A", A, small);
                if (op_selected == 1) DrawMatPreview("B", B, small);
                if (op_selected == 4) { DrawMatPreview("B (MatMul3)", B, small); DrawMatPreview("C (MatMul3)", C3, small); }
                if (op_selected == 0) DrawVecPreview("x", x, small);
                if (op_selected >= 2 && op_selected != 4) DrawVecPreview("b", b, small);

            }
            ImGui::End();
        }

        // --- PANELL: OPERACIO ---
        if (show_ops)
        {
            if (ImGui::Begin("Operacio", &show_ops))
            {
                const char* ops[] = {
                    "MatVec (A*x)", "MatMul (A*B)", "Solve (No Pivot)", "Solve (Partial Pivot)",
                    "MatMul3 (A*B*C)", "Solve (Partial Pivot, Augmented)"
                };
                ImGui::Combo("Operacio", &op_selected, ops, IM_ARRAYSIZE(ops));
                if (ImGui::Button("Run"))
                {
                    last_ops = OpsCounter{}; last_time_ms = 0.0; last_rel_res = std::numeric_limits<double>::quiet_NaN(); last_status = SolveStatus::None;
                    try
                    {
                        if (op_selected == 0)
                        {
                            // MatVec: A (m x nA) * x (nA) -> y (m)
                            if ((int)A.rows != m_rows || (int)A.cols != a_cols || (int)x.size() != a_cols) { last_status = SolveStatus::DimError; }
                            else {
                                Timer t; t.Tic();
                                y = A.Multiply(x, &last_ops);
                                last_time_ms = t.TocMs();
                                C = Matrix(); x_sol.clear();
                            }
                        }
                        else if (op_selected == 1)
                        {
                            // MatMul: A (m x nA) * B (nA x nB) -> C (m x nB)
                            if ((int)A.rows != m_rows || (int)A.cols != a_cols || (int)B.rows != a_cols || (int)B.cols != n_cols) { last_status = SolveStatus::DimError; }
                            else {
                                Timer t; t.Tic();
                                C = A.Multiply(B, &last_ops);
                                last_time_ms = t.TocMs();
                                y.clear(); x_sol.clear();
                            }
                        }
                        else if (op_selected == 2)
                        {
                            // Solve (No Pivot)
                            if ((int)A.rows != n_solve || (int)A.cols != n_solve || (int)b.size() != n_solve) { last_status = SolveStatus::DimError; }
                            else {
                                LinAlg::SolveReport r = LinAlg::SolveNoPivot(A, b, tol);
                                x_sol = r.x; last_time_ms = r.ms; last_ops = r.ops; last_rel_res = r.rel_resid;
                                last_status = r.pivot_zero ? SolveStatus::PivotFailure : SolveStatus::Ok;
                                y.clear(); C = Matrix();
                            }
                        }
                        else if (op_selected == 3)
                        {
                            // Solve (Partial Pivot)
                            if ((int)A.rows != n_solve || (int)A.cols != n_solve || (int)b.size() != n_solve) { last_status = SolveStatus::DimError; }
                            else {
                                LinAlg::SolveReport r = LinAlg::SolvePartialPivot(A, b, tol);
                                x_sol = r.x; last_time_ms = r.ms; last_ops = r.ops; last_rel_res = r.rel_resid;
                                last_status = r.singular ? SolveStatus::Singular : SolveStatus::Ok;
                                y.clear(); C = Matrix();
                            }
                        }
                        else if (op_selected == 4)
                        {
                            // MatMul3: A (m x nA) * B (nA x p) * C (p x q) -> C (m x q)
                            bool dims_ok =
                                (int)A.rows == m_rows && (int)A.cols == a_cols &&
                                (int)B.rows == a_cols && (int)B.cols == p_cols &&
                                (int)C3.rows == p_cols && (int)C3.cols == q_cols;
                            if (!dims_ok) { last_status = SolveStatus::DimError; }
                            else {
                                Timer t; t.Tic();
                                C = Multiply3(A, B, C3, &last_ops);
                                last_time_ms = t.TocMs();
                                y.clear(); x_sol.clear();
                            }
                        }
                        else if (op_selected == 5)
                        {
                            // Solve (Partial Pivot, Augmented): construeix Ab = [A|b]
                            if ((int)A.rows != n_solve || (int)A.cols != n_solve || (int)b.size() != n_solve) { last_status = SolveStatus::DimError; }
                            else {
                                Matrix Ab((size_t)n_solve, (size_t)n_solve + 1, 0.0);
                                for (int i = 0; i < n_solve; ++i) {
                                    for (int j = 0; j < n_solve; ++j) Ab.At((size_t)i, (size_t)j) = A.At((size_t)i, (size_t)j);
                                    Ab.At((size_t)i, (size_t)n_solve) = b[(size_t)i];
                                }
                                LinAlg::SolveReport r = LinAlg::SolvePivotAugmented(Ab, tol);
                                x_sol = r.x; last_time_ms = r.ms; last_ops = r.ops; last_rel_res = r.rel_resid;
                                last_status = r.singular ? SolveStatus::Singular : SolveStatus::Ok;
                                y.clear(); C = Matrix();
                            }
                        }
                    }
                    catch (const std::exception&)
                    {
                        ImGui::OpenPopup("error_popup");
                    }
                }

                if (ImGui::BeginPopupModal("error_popup", NULL, ImGuiWindowFlags_AlwaysAutoResize))
                {
                    ImGui::TextWrapped("%s", "S'ha produït una excepcio durant l'execucio (revisa dimensions i dades).");
                    ImGui::Separator();
                    if (op_selected == 0)
                        ImGui::TextDisabled("Esperat: A %dx%d, x %d", m_rows, a_cols, a_cols);
                    else if (op_selected == 1)
                        ImGui::TextDisabled("Esperat: A %dx%d, B %dx%d", m_rows, a_cols, a_cols, n_cols);
                    else if (op_selected == 4)
                        ImGui::TextDisabled("Esperat: A %dx%d, B %dx%d, C %dx%d", m_rows, a_cols, a_cols, p_cols, p_cols, q_cols);
                    else
                        ImGui::TextDisabled("Esperat: A %dx%d, b %d", n_solve, n_solve, n_solve);
                    if (ImGui::Button("Tanca")) ImGui::CloseCurrentPopup();
                    ImGui::EndPopup();
                }
                // Estat resolució
                switch (last_status) {
                case SolveStatus::DimError:
                    ImGui::SameLine(); ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "Dimensions incompatibles");
                    break;
                case SolveStatus::PivotFailure:
                    ImGui::SameLine(); ImGui::TextColored(ImVec4(1, 0.5f, 0.2f, 1), "(pivot~0 amb tol=%.1e)", tol);
                    break;
                case SolveStatus::Singular:
                    ImGui::SameLine(); ImGui::TextColored(ImVec4(1, 0.7f, 0.2f, 1), "Matriu singular (diag <= tol)");
                    break;
                default: break;
                }
            }
            ImGui::End();
        }

        // --- PANELL: RESULTATS ---
        if (show_results)
        {
            if (ImGui::Begin("Resultats", &show_results))
            {
                ImGui::Text("Temps (ms): %.3f", last_time_ms);
                if (std::isnan(last_rel_res)) ImGui::Text("Residu relatiu: N/A");
                else ImGui::Text("Residu relatiu: %.3e", last_rel_res);
                ImGui::Separator();
                ImGui::Text("Comptador d'operacions:");
                ImGui::BulletText("mul = %llu", (unsigned long long)last_ops.mul);
                ImGui::BulletText("add = %llu", (unsigned long long)last_ops.add);
                ImGui::BulletText("sub = %llu", (unsigned long long)last_ops.sub);
                ImGui::BulletText("div = %llu", (unsigned long long)last_ops.div_);
                ImGui::BulletText("cmp = %llu", (unsigned long long)last_ops.cmp);
                ImGui::BulletText("swp = %llu", (unsigned long long)last_ops.swp);

                ImGui::Separator();
                // Previews de resultats
                if (!y.empty()) DrawVecPreview("y = A*x", y, false);
                if (C.rows) DrawMatPreview("Resultat producte", C, false);
                if (!x_sol.empty()) DrawVecPreview("x (solucio)", x_sol, false);
            }
            ImGui::End();
        }

        // --- RENDER ---
        ImGui::Render();
        int dw, dh;
        SDL_GetWindowSizeInPixels(window, &dw, &dh);
        glViewport(0, 0, dw, dh);
        glClearColor(0.10f, 0.10f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    // Shutdown
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DestroyContext(gl_ctx);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}