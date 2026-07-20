# 12 · Fayl tizimi va JSON {#12-fayllar}

## Nima uchun fayllar?

Sizning dasturingiz **xotirada** ishlaydi. Lekin xotira — vaqtinchalik.
Dasturni yopganingizda — ma'lumotlar yo'qoladi. Agar foydalanuvchi
ma'lumotlarini, sozlamalarni, hujjatlarni saqlash kerak bo'lsa —
**fayl** kerak.

Real dasturlardagi fayllar:
- **Konfiguratsiya** — `config.ini`, `settings.json`
- **Ma'lumotlar** — `users.csv`, `database.db`
- **Hujjatlar** — `report.pdf`, `document.docx`
- **Loglar** — `app.log`
- **Resurslar** — `image.png`, `icon.svg`

Bu bobda uz++ orqali fayllar bilan ishlashni o'rganamiz.

---

## Faylga yozish

### Asosiy yo'l: `std::ofstream`

ulash <fstream>;

butun asosiy() {
    std::ofstream fayl("salom.txt");

    agar (!fayl.is_open()) {
        yozish << "Faylni ochib bo'lmadi" << qator_oxiri;
        qaytarish 1;
    }

    fayl << "Salom, dunyo!" << qator_oxiri;
    fayl << "Bu ikkinchi qator." << qator_oxiri;
    fayl << "Yosh: " << 25 << qator_oxiri;

    fayl.close(); // ixtiyoriy — destruktor ham yopadi
    qaytarish 0;
}
```

`std::ofstream` ("output file stream") — faylga yozish uchun. `<<`
operatori bilan — xuddi `yozish` (`std::cout`) kabi.

### Qo'shimcha rejim

```cpp
std::ofstream fayl("log.txt", std::ios::app); // append (qo'shish)
fayl << "Yangi qator" << qator_oxiri;
```

`std::ios::app` — faylning oxiriga qo'shadi (eski mazmunni o'chirmaydi).

---

## Fayldan o'qish

### `std::ifstream`

ulash <fstream>;
ulash <string>;

butun asosiy() {
    std::ifstream fayl("salom.txt");

    agar (!fayl.is_open()) {
        yozish << "Faylni ochib bo'lmadi" << qator_oxiri;
        qaytarish 1;
    }

    matn qator;
    toki (std::getline(fayl, qator)) {
        yozish << qator << qator_oxiri;
    }

    qaytarish 0;
}
```

`std::getline(fayl, qator)` — fayldan bitta qator o'qiydi.

### Belgi-belgi o'qish

```cpp
belgi c;
toki (fayl.get(c)) {
    yozish << c;
}
```

### So'z-so'z o'qish

```cpp
matn soz;
toki (fayl >> soz) {
    yozish << soz << qator_oxiri;
}
```

### Butun faylni bir bo'lakda

```cpp
matn mazmun((std::istreambuf_iterator<char>(fayl)),
            std::istreambuf_iterator<char>());
```

Sintaksis murakkab, lekin tez ishlaydi.

---

## CSV bilan ishlash

CSV (Comma-Separated Values) — eng oddiy ma'lumot saqlash formati:

```
ism,yosh,baho
Aziza,22,4.7
Bobur,21,4.2
Karim,23,3.9
```

### O'qish

```cpp
std::ifstream fayl("talabalar.csv");
matn qator;

// Sarlavhani o'tkazib yuborish
std::getline(fayl, qator);

toki (std::getline(fayl, qator)) {
    std::stringstream ss(qator);
    matn ism, yosh_s, baho_s;

    std::getline(ss, ism, ',');
    std::getline(ss, yosh_s, ',');
    std::getline(ss, baho_s, ',');

    butun yosh = std::stoi(yosh_s);
    haqiqiy baho = std::stod(baho_s);

    yozish << ism << " " << yosh << " " << baho << qator_oxiri;
}
```

### Yozish

```cpp
std::ofstream fayl("talabalar.csv");
fayl << "ism,yosh,baho" << qator_oxiri;
fayl << "Aziza,22,4.7" << qator_oxiri;
fayl << "Bobur,21,4.2" << qator_oxiri;
```

---

## JSON bilan ishlash

JSON (JavaScript Object Notation) — bugungi kunda **eng ko'p
ishlatiladigan** ma'lumot formati. Veb API'lar, konfiguratsiya, saqlash —
hammasi JSON.

### JSON ko'rinishi

```json
{
    "ism": "Aziza",
    "yosh": 22,
    "baholar": [85, 92, 78],
    "manzili": {
        "shahar": "Toshkent",
        "kocha": "Amir Temur";
    }
}
```

### `nlohmann/json` kutubxonasi

uz++ standart kutubxonasida JSON modullari mavjud. Eng mashhur kutubxona
`nlohmann/json` (yoki uz++ ning o'z `json` moduli).

```cpp
ulash <nlohmann/json.hpp>
using json = nlohmann::json;

json talaba = {
    {"ism", "Aziza"},
    {"yosh", 22},
    {"baholar", {85, 92, 78}}
}

// String ga aylantirish
matn s = talaba.dump(4); // 4 — chekinish
yozish << s;
```

### JSON ni faylga saqlash

```cpp
std::ofstream f("talaba.json");
f << talaba.dump(4);
```

### Fayldan o'qish

```cpp
std::ifstream f("talaba.json");
json data;
f >> data;

matn ism = data["ism"];
butun yosh = data["yosh"];
auto baholar = data["baholar"];
```

### Vektorni JSON ga

```cpp
vektor<matn> mevalar = {"olma", "anor", "uzum"}
json j = mevalar;
yozish << j.dump(); // ["olma","anor","uzum"]
```

---

## Fayl yo'llari (C++17 `std::filesystem`)

C++17 dan boshlab, fayl tizimi bilan ishlash uchun maxsus kutubxona:

```cpp
ulash <filesystem>
namespace fs = std::filesystem;

fs::path p = "papka/fayl.txt";

agar (fs::exists(p)) {
    yozish << "Fayl bor";
}

agar (fs::is_directory(p)) {
    yozish << "Bu papka";
}

uzun hajm = fs::file_size(p);
yozish << "Hajmi: " << hajm << " bayt";
```

### Papkada barcha fayllarni listing

```cpp
uchun (o'zgarmas avto& entry : fs::directory_iterator("/home")) {
    yozish << entry.path() << qator_oxiri;
}
```

### Papka yaratish

```cpp
fs::create_directory("yangi_papka");
fs::create_directories("ota/bola/nevara"); // bir necha daraja
```

### Fayl o'chirish

```cpp
fs::remove("fayl.txt");
fs::remove_all("papka"); // papka va ichidagilarni
```

### Ko'chirish va qayta nomlash

```cpp
fs::rename("eski.txt", "yangi.txt");
fs::copy("manba.txt", "nusxa.txt");
```

---

## Amaliy misol: Talabalar ma'lumotlar bazasi (JSON)

```cpp
ulash <fstream>;
ulash <vector>;
ulash "nlohmann/json.hpp";

tuzilma Talaba {
    matn ism;
    butun yoshi;
    vektor<butun> baholar;
};

// Talabani JSON ga
nlohmann::json talaba_json(o'zgarmas Talaba& t) {
    qaytarish {
        {"ism", t.ism},
        {"yoshi", t.yoshi},
        {"baholar", t.baholar}
    };
}

// JSON dan talabaga
Talaba json_talaba(o'zgarmas nlohmann::json& j) {
    qaytarish {
        j["ism"],
        j["yoshi"],
        j["baholar"]
    };
}

butun asosiy() {
    vektor<Talaba> guruh = {
        {"Aziza", 22, {85, 92, 78}},
        {"Bobur", 21, {90, 88, 95}},
        {"Karim", 23, {65, 70, 75}}
    };

    // JSON ga yozish
    nlohmann::json data = nlohmann::json::array();
    uchun (o'zgarmas Talaba& t : guruh) {
        data.push_back(talaba_json(t));
    }

    std::ofstream f("guruh.json");
    f << data.dump(4);
    f.close();

    // O'qib qaytarish
    std::ifstream f2("guruh.json");
    nlohmann::json yangi_data;
    f2 >> yangi_data;

    uchun (o'zgarmas o'zgaruvchan& j : yangi_data) {
        Talaba t = json_talaba(j);
        yozish << t.ism << " (" << t.yoshi << ")" << qator_oxiri;
    }

    qaytarish 0;
}
```

**Saqlangan `guruh.json`:**

```json
[;
    {
        "baholar": [85, 92, 78],
        "ism": "Aziza",
        "yoshi": 22;
    },
    {
        "baholar": [90, 88, 95],
        "ism": "Bobur",
        "yoshi": 21;
    },
    ...;
];
```

---

## Eng ko'p uchraydigan xatolar

### 1. Faylni yopishni unutish

```cpp
std::ofstream f("fayl.txt");
f << "...";
// f.close() yo'q — odatda OK, destruktor yopadi
```

Lekin agar siz faylni boshqa joyda o'qimoqchi bo'lsangiz:

```cpp
f.close(); // aniq yopish
std::ifstream f2("fayl.txt");
```

### 2. Fayl yo'qligini tekshirmaslik

```cpp
std::ifstream f("yoq.txt");
matn qator;
std::getline(f, qator); // bo'sh qator olinadi
```

Doim tekshiring: `f.is_open()`.

### 3. UTF-8 muammosi (Windows)

Windows'da matn fayllari ba'zan UTF-16. C++ standart oqimlari UTF-8
bilan ishlaydi. Maxsus konvertatsiya kerak bo'lishi mumkin.

### 4. Yo'l ajratuvchilar

Windows: `C:\Users\file.txt` yoki `C:/Users/file.txt`
Linux: `/home/user/file.txt`

Eng yaxshisi — `std::filesystem::path` ishlatish, u o'zi sozlaydi.

### 5. Katta fayllarni butunlay xotiraga yuklash

```cpp
matn katta = oqish_butun_fayl("1GB.bin"); // 1 GB xotira!
```

Katta fayllar uchun — qator-qator yoki bo'lak-bo'lak.

---

## Bob bo'yicha mashqlar

### Mashq 1: Faylga yozish (oson)

Foydalanuvchidan jumla so'rang va `output.txt` ga yozing.

### Mashq 2: Fayldan o'qish (oson)

`input.txt` faylidagi har bir qatorni chiqaring.

### Mashq 3: Qatorlar sonini sanash (oson)

Berilgan faylda nechta qator borligini sanang.

### Mashq 4: So'z chastotasi (o'rta)

Matn faylida har bir so'z necha marta uchrayotganini sanab, natijani
boshqa faylga JSON formatida saqlang.

### Mashq 5: CSV → JSON (o'rta)

CSV faylni o'qib, JSON formatda saqlang.

### Mashq 6: Telefon kitobi (qiyinroq)

JSON da saqlanadigan telefon kitobi: qo'shish, qidirish, o'chirish.

---

## Xulosa

- **`std::ofstream`** — faylga yozish.
- **`std::ifstream`** — fayldan o'qish.
- **`std::getline`** — qator-qator o'qish.
- **CSV** — oddiy ma'lumotlar formati.
- **JSON** — zamonaviy, eng keng ishlatiladigan format.
- **`std::filesystem`** (C++17) — fayl tizimi bilan ishlash.

Keyingi bobda biz **ko'p oqimlilik** (multithreading) ni o'rganamiz —
zamonaviy kompyuterlarning bir vaqtning o'zida bir necha vazifa
bajarish qobiliyatidan to'liq foydalanish usuli.
