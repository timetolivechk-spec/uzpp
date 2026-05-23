# 15 · GUI dasturlash (CopperSpice) {#15-gui}

## GUI nima va nima uchun kerak?

Hozirgacha bizning dasturlarimiz **konsol** (terminal) orqali ishlardi.
Foydalanuvchi matn yozadi, dastur matn chiqaradi. Bu — yaxshi, lekin
zamonaviy foydalanuvchi **grafik interfeysni** (GUI — Graphical User
Interface) kutadi: tugmalar, oynalar, menyu, slaydlar.

GUI dasturlar:
- Veb-brauzer (Chrome, Firefox)
- Hujjat muharrirlari (Word, Notepad++)
- Suratlar tahrir qiluvchilar (Photoshop, GIMP)
- O'yinlar
- Operatsion tizim oyna boshqaruvchisi

uz++ da GUI yaratish uchun biz **CopperSpice** kutubxonasidan
foydalanamiz. Bu Qt'ning ochiq versiyasi.

> **Diqqat:** Bu bob faqat tushuncha berish uchun. CopperSpice'ni
> o'rnatish va sozlash alohida hujjatlarda batafsil bayon etilgan.

---

## Birinchi GUI dastur

```cpp
// salom_oyna.uzpp
#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

butun asosiy(butun argc, belgi* argv[]) {
    QApplication dastur(argc, argv)

    QWidget oyna
    oyna.setWindowTitle("Birinchi GUI dasturim")
    oyna.resize(400, 300)

    QLabel* matn = new QLabel("Salom, dunyo!")
    QPushButton* tugma = new QPushButton("Bosing!")

    QVBoxLayout* maket = new QVBoxLayout(&oyna)
    maket->addWidget(matn)
    maket->addWidget(tugma)

    oyna.show()
    qaytarish dastur.exec()
}
```

Ishga tushirish:

```bash
uzpp qurish salom_oyna.uzpp --bare -I C:/CopperSpice/include -l QtCore -l QtGui
```

`--bare` — uz++ runtime'siz qurish. `-I` va `-l` — CopperSpice
sarlavhalari va kutubxonalari.

### Bu dasturda nima bor?

1. **`QApplication`** — har bir GUI dasturning asosi. Hodisalarni
   boshqaradi.
2. **`QWidget`** — asosiy oyna.
3. **`QLabel`** — matn ko'rsatuvchi vidjet.
4. **`QPushButton`** — tugma.
5. **`QVBoxLayout`** — vertikal joylashuv (yuqoridan pastga).
6. **`oyna.show()`** — oynani ko'rsatish.
7. **`dastur.exec()`** — hodisalar tsiklini ishga tushirish.

---

## Maketlar (Layouts)

Vidjetlarni oynaga joylashish — _layout_ orqali:

### Vertikal va gorizontal

```cpp
QVBoxLayout                            // vertikal
QHBoxLayout                            // gorizontal
```

### Grid (panjara)

```cpp
QGridLayout* maket = new QGridLayout()
maket->addWidget(vidjet1, 0, 0)        // qator 0, ustun 0
maket->addWidget(vidjet2, 0, 1)        // qator 0, ustun 1
maket->addWidget(vidjet3, 1, 0)        // qator 1, ustun 0
```

### Forma

```cpp
QFormLayout* maket = new QFormLayout()
maket->addRow("Ism:", new QLineEdit())
maket->addRow("Yosh:", new QSpinBox())
```

---

## Signallar va slotlar

CopperSpice'ning eng kuchli xususiyati — **signal-slot** tizimi.
Tugma bosilganda biror funksiyani chaqirish uchun:

```cpp
QPushButton* tugma = new QPushButton("Bosing!")

// Lambda bilan
QObject::connect(tugma, &QPushButton::clicked, [](){
    yozish << "Tugma bosildi!" << qator_oxiri
})

// Yoki funksiya bilan
bosh tugma_bosildi() {
    yozish << "Tugma bosildi!" << qator_oxiri
}
QObject::connect(tugma, &QPushButton::clicked, tugma_bosildi)
```

Signal — vidjet "men hodisani his qildim" deydi.
Slot — funksiya "men buni qabul qilaman" deydi.

### Misol: Hisoblagich

```cpp
butun hisoblagich = 0
QLabel* yorliq = new QLabel("0")
QPushButton* tugma = new QPushButton("Oshir")

QObject::connect(tugma, &QPushButton::clicked, [&]() {
    hisoblagich++
    yorliq->setText(std::to_string(hisoblagich).c_str())
})
```

Har safar tugma bosilganda — hisoblagich oshadi va yorliqda yangilanadi.

---

## Muhim vidjetlar

| Vidjet | Maqsadi |
|--------|---------|
| `QLabel` | Matn yoki rasm ko'rsatish |
| `QPushButton` | Tugma |
| `QLineEdit` | Bir qatorlik matn maydoni |
| `QTextEdit` | Ko'p qatorli matn |
| `QCheckBox` | Belgilash kutisi |
| `QRadioButton` | Radio tanlov |
| `QComboBox` | Pastga tushadigan ro'yxat |
| `QSpinBox` | Son tanlash |
| `QSlider` | Slayder |
| `QProgressBar` | Progress indikatori |
| `QListWidget` | Ro'yxat |
| `QTableWidget` | Jadval |
| `QTreeWidget` | Daraxt |

### Misol: matn maydonidan o'qish

```cpp
QLineEdit* matn_maydoni = new QLineEdit()
QPushButton* tugma = new QPushButton("Tasdiqlash")

QObject::connect(tugma, &QPushButton::clicked, [matn_maydoni]() {
    QString matn = matn_maydoni->text()
    yozish << "Foydalanuvchi yozdi: " << matn.toStdString()
})
```

---

## Menyu va status qatori

```cpp
QMainWindow oyna

// Menyu yaratish
QMenu* fayl_menyu = oyna.menuBar()->addMenu("Fayl")
fayl_menyu->addAction("Yangi")
fayl_menyu->addAction("Ochish")
fayl_menyu->addSeparator()
fayl_menyu->addAction("Chiqish")

// Status qatori
oyna.statusBar()->showMessage("Tayyor")
```

---

## To'liq misol: Mini matn muharriri

```cpp
#include <QApplication>
#include <QMainWindow>
#include <QTextEdit>
#include <QMenu>
#include <QMenuBar>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>

butun asosiy(butun argc, belgi* argv[]) {
    QApplication dastur(argc, argv)

    QMainWindow oyna
    oyna.setWindowTitle("Mini muharrir")
    oyna.resize(800, 600)

    QTextEdit* muharrir = new QTextEdit()
    oyna.setCentralWidget(muharrir)

    // Menyu
    QMenu* fayl_menyu = oyna.menuBar()->addMenu("Fayl")

    QAction* yangi = fayl_menyu->addAction("Yangi")
    QObject::connect(yangi, &QAction::triggered, [muharrir]() {
        muharrir->clear()
    })

    QAction* ochish = fayl_menyu->addAction("Ochish")
    QObject::connect(ochish, &QAction::triggered, [&]() {
        QString yol = QFileDialog::getOpenFileName(&oyna, "Faylni ochish")
        agar (!yol.isEmpty()) {
            QFile f(yol)
            agar (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream oqim(&f)
                muharrir->setPlainText(oqim.readAll())
            }
        }
    })

    QAction* saqlash = fayl_menyu->addAction("Saqlash")
    QObject::connect(saqlash, &QAction::triggered, [&]() {
        QString yol = QFileDialog::getSaveFileName(&oyna, "Saqlash")
        agar (!yol.isEmpty()) {
            QFile f(yol)
            agar (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream oqim(&f)
                oqim << muharrir->toPlainText()
            }
        }
    })

    fayl_menyu->addSeparator()

    QAction* chiqish = fayl_menyu->addAction("Chiqish")
    QObject::connect(chiqish, &QAction::triggered, &dastur, &QApplication::quit)

    oyna.show()
    qaytarish dastur.exec()
}
```

Bu — to'liq ishlaydigan matn muharriri! Fayl ochish, saqlash, yangi.

---

## `--bare` rejimi

CopperSpice/Qt bilan ishlash uchun uz++ ning maxsus rejimi:

```bash
uzpp qurish dastur.uzpp --bare -I /path/to/coperspice/include -l QtCore -l QtGui
```

`--bare`:
- Standart uz++ runtime'siz qurish
- Tashqi kutubxonalar bilan integratsiya uchun
- Sof C++ asosida

---

## Bob bo'yicha mashqlar

### Mashq 1: Salom dunyo oynasi (oson)

Oddiy "Salom, dunyo!" yorlig'i bo'lgan oyna yarating.

### Mashq 2: Kalkulyator (o'rta)

GUI kalkulyator: ikkita son, amal tugmalari, natija.

### Mashq 3: Soat (o'rta)

`QTimer` bilan har soniya yangilanadigan soat ko'rsating.

### Mashq 4: To'do ro'yxati (o'rtacha)

`QListWidget` bilan ishlar ro'yxati. Qo'shish, o'chirish tugmalari.

### Mashq 5: Suratlar ko'rsatkichi (qiyinroq)

Papkadan suratlarni o'qib `QLabel` da ko'rsating. "Oldingi" va
"Keyingi" tugmalari bilan.

---

## Xulosa

- **GUI** — grafik interfeys, zamonaviy dasturlarning standarti.
- **CopperSpice** (yoki Qt) — uz++ uchun asosiy GUI kutubxonasi.
- **`QApplication`** — har bir GUI dasturning asosi.
- **Vidjetlar** — `QLabel`, `QPushButton`, `QLineEdit`, va h.k.
- **Maketlar** — vidjetlarni joylashtirish.
- **Signal va slot** — hodisa-funksiya bog'lanishi.
- **`--bare`** — uz++ ning maxsus GUI rejimi.

Keyingi bobda **ilovalar** bor — kalit so'zlar lug'ati, o'rnatish
bo'yicha to'liq yo'riqnoma, foydali resurslar.
