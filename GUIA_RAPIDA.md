# 🚀 GUIA RÀPIDA - Lab1 per a Companys de la Uni

## ❓ Què fa aquest codi?

Resol sistemes d'equacions lineals **Ax = b** com els que apareixen quan combines recursos en un RPG.

---

## 📁 Estructura del Projecte

```
src/
├── Matrix.cpp   ← Exercici 1: Operacions amb matrius
├── LinAlg.cpp   ← Exercici 2: Normes i residus
└── Solve.cpp    ← Exercicis 2 i 3: Resoldre sistemes
```

---

## 🎯 Exercici 1: Operacions Bàsiques

### Què s'ha fet?

✅ **Matrix::Identity(n)** → Crea matriu identitat (1's a la diagonal)
✅ **Matrix::At(i,j)** → Accedeix a l'element [fila i, columna j]
✅ **Matrix::Multiply(x)** → Producte matriu-vector (Ax)
✅ **Matrix::Multiply(B)** → Producte matriu-matriu (AB)

### Complexitat

| Operació | Temps | FLOPs |
|----------|-------|-------|
| **Ax** | O(n²) | 2n² |
| **AB** | O(n³) | 2n³ |

### Exemple d'ús

```cpp
Matrix A(3, 3);
A.At(0, 0) = 1;  // Assigna valor
Vec x = {1, 2, 3};
Vec y = A.Multiply(x);  // y = Ax
```

---

## 🔧 Exercici 2: Sense Pivotatge

### Què s'ha fet?

✅ **GaussianElimination** → Transforma Ax=b en Ux=c (triangular superior)
✅ **BackSubstitution** → Resol Ux=c de baix a dalt
✅ **SolveNoPivot** → Combina els dos passos anteriors

### Com funciona?

**1. Eliminació Gaussiana:**
```
[2 1 1 | 5]      [2 1  1  | 5 ]
[3 2 3 | 8]  →   [0 0.5 1.5| 0.5]  (triangular superior)
[1 2 4 | 7]      [0 0  -1  | 3 ]
```

**2. Substitució Enrere:**
```
x₃ = 3 / (-1) = -3
x₂ = (0.5 - 1.5·(-3)) / 0.5 = 9.5
x₁ = (5 - 1·9.5 - 1·(-3)) / 2 = -0.75
```

### ⚠️ Limitació
Pot fallar si hi ha pivots petits (elements diagonals ≈ 0).

---

## 🎯 Exercici 3: Amb Pivotatge

### Què s'ha fet?

✅ **GaussianEliminationPivot** → Igual que abans PERÒ busca el pivot més gran
✅ **SolvePartialPivot** → Versió millorada de SolveNoPivot

### Per què és millor?

**Sense pivotatge:**
```
[0.0001  1] [x₁]   [1]
[  1     1] [x₂] = [2]

Pivot petit (0.0001) → ERRORS ENORMES! ❌
```

**Amb pivotatge:**
```
Intercanviem files:
[  1     1] [x₁]   [2]
[0.0001  1] [x₂] = [1]

Pivot gran (1) → Tot OK! ✅
```

### Diferència clau

| Sense Pivotatge | Amb Pivotatge |
|----------------|---------------|
| Ràpid ⚡ | Lleugerament més lent ⚡ |
| Inestable ❌ | Estable ✅ |
| Taxa èxit: ~60% | Taxa èxit: ~99% |
| **NO recomanat** | **RECOMANAT** ✅ |

---

## 📊 Comptatge d'Operacions (Important!)

### Per què comptem operacions?

- Verificar que el codi és correcte
- Comparar amb la teoria
- Analitzar l'eficiència

### Què comptem?

✅ **Multiplicacions** (`op->IncMul()`)
✅ **Sumes/Restes** (`op->IncAdd()`, `op->IncSub()`)
✅ **Divisions** (`op->IncDiv()`)
✅ **Comparacions** (`op->IncCmp()`) ← **IMPORTANT: No t'oblidis!**
✅ **Swaps** (`op->IncSwp()`) ← **2 per pivot: A + b**

### Exemple

```cpp
// ❌ MALAMENT (falta comptar):
if (std::abs(pivot) <= tol) { ... }

// ✅ BÉ (compta la comparació):
if (op) op->IncCmp();
if (std::abs(pivot) <= tol) { ... }
```

---

## 🔍 Com Verificar que Està Correcte?

### 1️⃣ Comptatge FLOPs

Executa el banc de proves:
```bash
./main_bench
```

Hauries de veure:
```
✓ Mat-Vec (n=500): 499,500 FLOPs (dins del marge)
✓ Mat-Mat (n=500): 249,500,000 FLOPs (dins del marge)
✓ EG (n=500): ~41M FLOPs (dins del marge)
✓ SE (n=500): ~125K FLOPs (dins del marge)
```

### 2️⃣ Residu Relatiu

Un residu petit significa que la solució és bona:
```
✓ < 10⁻¹⁰  → Perfecte!
✓ < 10⁻⁶   → Bé
⚠ < 10⁻³   → Dubtós
✗ > 10⁻³   → Malament!
```

### 3️⃣ Escalat Temporal

Els temps han de créixer com:
- **Ax:** O(n²) → temps × 4 quan n × 2
- **AB:** O(n³) → temps × 8 quan n × 2
- **Solve:** O(n³) → temps × 8 quan n × 2

---

## 🚨 Errors Comuns i Solucions

### ❌ Error 1: No comptar comparacions amb tolerància
```cpp
// MALAMENT:
if (std::abs(pivot) <= tol) { return false; }

// BÉ:
if (op) op->IncCmp();  // ← Afegeix això!
if (std::abs(pivot) <= tol) { return false; }
```

### ❌ Error 2: Comptar només 1 swap en pivotatge
```cpp
// MALAMENT:
A.SwapRows(k, pivot_row);
std::swap(b[k], b[pivot_row]);
if (op) op->IncSwp();  // ← Només 1!

// BÉ:
A.SwapRows(k, pivot_row);
std::swap(b[k], b[pivot_row]);
if (op) {
    op->IncSwp();  // Swap d'A
    op->IncSwp();  // Swap de b
}
```

### ❌ Error 3: Comprovar diagonal == 0 a BackSubstitution
```cpp
// MALAMENT (no cal fer-ho!):
if (U.At(i, i) == 0.0) { ... }

// BÉ (elimina-ho):
// No cal comprovar, BackSubstitution només es crida si el sistema té solució
```

### ❌ Error 4: No usar At(i,j)
```cpp
// MALAMENT (menys llegible):
double val = data[i * cols + j];

// BÉ (més clar):
double val = A.At(i, j);
```

---

## 📝 Checklist Final

Abans d'entregar, comprova:

- [ ] **Codi compila sense errors**
- [ ] **Banc de proves PASS per tots els tests**
- [ ] **Comptatge FLOPs dins del marge (±10%)**
- [ ] **Comparacions amb tolerància comptades**
- [ ] **2 swaps per pivot (A + b)**
- [ ] **Residu relatiu < 10⁻¹⁰**
- [ ] **Codi usa At(i,j) per llegibilitat**
- [ ] **NO comprova diagonal==0 a BackSubstitution**
- [ ] **Informe PDF amb deduccions teòriques**
- [ ] **Captures de pantalla amb explicacions**

---

## 💡 Consells per a l'Informe

### Estructura (2-4 pàgines)

**1. Introducció (1/2 pàgina)**
- Context del problema (RPG amb recursos)
- Objectius de cada exercici

**2. Exercici 1 (1 pàgina)**
- Deducció: O(mn) per Ax, O(mnp) per AB
- Taula: n, FLOPs teòrics, FLOPs mesurats, temps
- Gràfic: log(temps) vs log(n) → pendent 2 (Ax) i 3 (AB)

**3. Exercici 2 (1 pàgina)**
- Deducció: n³/3 + n²/2 (EG), n²/2 + n (SE)
- Taula: n, FLOPs, comparacions, temps, residu
- Explicació: Per què residu < 10⁻¹⁰ és bo

**4. Exercici 3 (1 pàgina)**
- Deducció: Comparacions extra + swaps
- Taula: Comparació amb/sense pivot
- Anàlisi: Millora d'estabilitat numèrica

**5. Conclusions (1/4 pàgina)**
- Resum de resultats
- Avantatges del pivotatge

---

## 🎓 Preguntes Freqüents

**P: Per què no cal comprovar diagonal==0 a BackSubstitution?**
R: Perquè aquesta funció només es crida després de GaussianElimination, que ja ha comprovat que el sistema té solució.

**P: Per què comptem 2 swaps en pivotatge?**
R: Perquè quan pivotes, intercanvies 2 coses: una fila d'A i un element de b.

**P: Quin mètode usar: SolveNoPivot o SolvePartialPivot?**
R: Sempre SolvePartialPivot en aplicacions reals (més estable).

**P: Com sé si el residu és bo?**
R: < 10⁻¹⁰ és perfecte, < 10⁻⁶ és acceptable, > 10⁻³ és dubtós.

**P: Els temps han de coincidir exactament amb la teoria?**
R: No exactament, però l'escalat sí (Ax → O(n²), AB → O(n³)).

---

## 📚 Arxius Importants

```
src/
├── Matrix.cpp     ← Implementació matrius
├── LinAlg.cpp     ← Normes i residus
└── Solve.cpp      ← Algorismes principals

include/
├── Matrix.hpp     ← Declaracions
├── LinAlg.hpp     
└── Solve.hpp      

datasets/          ← Dades de prova
├── A_500.bin
├── b_500.bin
└── ...

app/
└── main_app.cpp   ← Interfície gràfica

bench/
└── main_bench.cpp ← Tests automàtics
```

---

## 🎯 Nota Esperada

Si segueixis aquesta guia i el checklist:
- **Codi correcte:** 8.5/10
- **Codi + informe bé fet:** 9.5/10
- **Codi perfecte + informe excel·lent:** 10/10 ✨

---

## 🤝 Crèdits

**Codi:** Zakaria Hamdaoui  
**Data:** Octubre 2025  

---

**Bona sort amb la pràctica! 🚀**

Si tens dubtes, revisa els comentaris al codi (estan super detallats) o consulta DOCUMENTACIO_COMPLETA.md.
