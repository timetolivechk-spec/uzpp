#pragma once

#include "platforma.hpp"
#include "xavfsizlik.hpp"
#include "xatoliklar.hpp"
#include "asinxron.hpp"

#include <sqlite3.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <utility>
#include <variant>
#include <sstream>

#if __has_include(<mysql/mysql.h>)
#include <mysql/mysql.h>
#endif

namespace uzpp::Baza {

using Qator = std::unordered_map<std::string, std::string>;
using Qiymat = std::variant<std::nullptr_t, int, double, std::string>;

class SorovQuruvchi; // Forward declaration

class Ulanish {
private:
    struct SQLiteDeleter {
        void operator()(sqlite3* db) const {
            if (db) sqlite3_close(db);
        }
    };
    
    std::shared_ptr<sqlite3> db_;

    explicit Ulanish(sqlite3* db) : db_(db, SQLiteDeleter{}) {}

public:
    // Default constructor for value semantics in expected
    Ulanish() : db_(nullptr) {}

    static Asinxron::Vazifa<Xavfsizlik::Natija<Ulanish, std::string>> ochish(const std::string& faylNomi) {
        sqlite3* db = nullptr;
        int rc = sqlite3_open(faylNomi.c_str(), &db);
        if (rc != SQLITE_OK) {
            std::string err = db ? sqlite3_errmsg(db) : "Noma'lum xato";
            if (db) sqlite3_close(db);
            co_return Xavfsizlik::xato("Baza ochilmadi: " + err);
        }
        co_return Xavfsizlik::muvaffaqiyat(Ulanish(db));
    }

    struct StmtDeleter {
        void operator()(sqlite3_stmt* stmt) const {
            if (stmt) sqlite3_finalize(stmt);
        }
    };

    // Helper for bindings
    static void bindValue(sqlite3_stmt* stmt, int index, int value) {
        sqlite3_bind_int(stmt, index, value);
    }
    static void bindValue(sqlite3_stmt* stmt, int index, double value) {
        sqlite3_bind_double(stmt, index, value);
    }
    static void bindValue(sqlite3_stmt* stmt, int index, const std::string& value) {
        sqlite3_bind_text(stmt, index, value.c_str(), -1, SQLITE_TRANSIENT);
    }
    static void bindValue(sqlite3_stmt* stmt, int index, const char* value) {
        sqlite3_bind_text(stmt, index, value, -1, SQLITE_TRANSIENT);
    }
    static void bindValue(sqlite3_stmt* stmt, int index, std::nullptr_t) {
        sqlite3_bind_null(stmt, index);
    }
    
    // Base case for fold-expressions
    static void bindAll(sqlite3_stmt* /*stmt*/, int /*index*/) {}

    template<typename T, typename... Args>
    static void bindAll(sqlite3_stmt* stmt, int index, T&& first, Args&&... rest) {
        bindValue(stmt, index, std::forward<T>(first));
        bindAll(stmt, index + 1, std::forward<Args>(rest)...);
    }

    template<typename... Args>
    Asinxron::Vazifa<Xavfsizlik::Natija<std::vector<Qator>, std::string>> bajarish(const std::string& sql, Args&&... args) {
        if (!db_) {
            co_return Xavfsizlik::xato(std::string("Baza ulanishi yopiq"));
        }

        sqlite3_stmt* rawStmt = nullptr;
        int rc = sqlite3_prepare_v2(db_.get(), sql.c_str(), -1, &rawStmt, nullptr);
        if (rc != SQLITE_OK) {
            co_return Xavfsizlik::xato(
                std::string("So'rov tayyorlanmadi: ") + std::string(sqlite3_errmsg(db_.get()))
            );
        }

        std::unique_ptr<sqlite3_stmt, StmtDeleter> stmt(rawStmt);

        bindAll(stmt.get(), 1, std::forward<Args>(args)...);

        std::vector<Qator> natijalar;
        while ((rc = sqlite3_step(stmt.get())) == SQLITE_ROW) {
            Qator qator;
            int colCount = sqlite3_column_count(stmt.get());
            for (int i = 0; i < colCount; ++i) {
                const char* colName = sqlite3_column_name(stmt.get(), i);
                const char* colText = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), i));
                qator[colName] = colText ? colText : "";
            }
            natijalar.push_back(std::move(qator));
        }

        if (rc != SQLITE_DONE) {
            co_return Xavfsizlik::xato(
                std::string("So'rov bajarilishida xato: ") + std::string(sqlite3_errmsg(db_.get()))
            );
        }

        co_return Xavfsizlik::muvaffaqiyat(std::move(natijalar));
    }

    // Dinamik parametrlarni qabul qiluvchi pastki darajadagi API (ORM uchun)
    Asinxron::Vazifa<Xavfsizlik::Natija<std::vector<Qator>, std::string>> bajarishDinamik(const std::string& sql, const std::vector<Qiymat>& parametrlar) {
        if (!db_) co_return Xavfsizlik::xato(std::string("Baza ulanishi yopiq"));

        sqlite3_stmt* rawStmt = nullptr;
        int rc = sqlite3_prepare_v2(db_.get(), sql.c_str(), -1, &rawStmt, nullptr);
        if (rc != SQLITE_OK) {
            co_return Xavfsizlik::xato("So'rov tayyorlanmadi: " + std::string(sqlite3_errmsg(db_.get())));
        }

        std::unique_ptr<sqlite3_stmt, StmtDeleter> stmt(rawStmt);

        for (size_t i = 0; i < parametrlar.size(); ++i) {
            std::visit([&](auto&& arg) { bindValue(stmt.get(), static_cast<int>(i + 1), arg); }, parametrlar[i]);
        }

        std::vector<Qator> natijalar;
        while ((rc = sqlite3_step(stmt.get())) == SQLITE_ROW) {
            Qator qator;
            int colCount = sqlite3_column_count(stmt.get());
            for (int i = 0; i < colCount; ++i) {
                const char* colName = sqlite3_column_name(stmt.get(), i);
                const char* colText = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), i));
                qator[colName] = colText ? colText : "";
            }
            natijalar.push_back(std::move(qator));
        }

        if (rc != SQLITE_DONE) {
            co_return Xavfsizlik::xato("So'rov bajarilishida xato: " + std::string(sqlite3_errmsg(db_.get())));
        }

        co_return Xavfsizlik::muvaffaqiyat(std::move(natijalar));
    }

    // Ma'lumot qaytarmaydigan ijro (Insert, Update, Delete)
    Asinxron::Vazifa<Xavfsizlik::Natija<int, std::string>> ijroDinamik(const std::string& sql, const std::vector<Qiymat>& parametrlar) {
        if (!db_) co_return Xavfsizlik::xato(std::string("Baza ulanishi yopiq"));

        sqlite3_stmt* rawStmt = nullptr;
        int rc = sqlite3_prepare_v2(db_.get(), sql.c_str(), -1, &rawStmt, nullptr);
        if (rc != SQLITE_OK) {
            co_return Xavfsizlik::xato("So'rov tayyorlanmadi: " + std::string(sqlite3_errmsg(db_.get())));
        }

        std::unique_ptr<sqlite3_stmt, StmtDeleter> stmt(rawStmt);

        for (size_t i = 0; i < parametrlar.size(); ++i) {
            std::visit([&](auto&& arg) { bindValue(stmt.get(), static_cast<int>(i + 1), arg); }, parametrlar[i]);
        }

        rc = sqlite3_step(stmt.get());
        if (rc != SQLITE_DONE) {
            co_return Xavfsizlik::xato("Ijro xatosi: " + std::string(sqlite3_errmsg(db_.get())));
        }

        co_return Xavfsizlik::muvaffaqiyat(sqlite3_changes(db_.get()));
    }

    int oxirgiKiritilganId() const {
        return db_ ? static_cast<int>(sqlite3_last_insert_rowid(db_.get())) : 0;
    }
    
    inline SorovQuruvchi jadval(const std::string& nomi);
};

// ===== MIGRATSIYA QURUVCHI =====
class JadvalQuruvchi {
    std::string nomi_;
    std::vector<std::string> ustunlar_;
public:
    explicit JadvalQuruvchi(std::string nomi) : nomi_(std::move(nomi)) {}
    
    JadvalQuruvchi& id() { ustunlar_.push_back("id INTEGER PRIMARY KEY AUTOINCREMENT"); return *this; }
    JadvalQuruvchi& matn(const std::string& nom) { ustunlar_.push_back(nom + " TEXT"); return *this; }
    JadvalQuruvchi& butun(const std::string& nom) { ustunlar_.push_back(nom + " INTEGER"); return *this; }
    JadvalQuruvchi& haqiqiy(const std::string& nom) { ustunlar_.push_back(nom + " REAL"); return *this; }
    JadvalQuruvchi& mantiqiy(const std::string& nom) { ustunlar_.push_back(nom + " BOOLEAN"); return *this; }
    
    std::string yigishSql() const {
        std::string sql = "CREATE TABLE IF NOT EXISTS " + nomi_ + " (";
        for(size_t i = 0; i < ustunlar_.size(); i++) {
            if (i > 0) sql += ", ";
            sql += ustunlar_[i];
        }
        sql += ");";
        return sql;
    }
};

// ===== ASINXRON ORM =====
class SorovQuruvchi {
    Ulanish* db_;
    std::string jadval_;
    std::string tanlovlar_ = "*";
    std::string shart_;
    std::vector<Qiymat> shartParametrlari_;
    std::string tartib_;
    int limit_ = -1;
    int offset_ = -1;

public:
    SorovQuruvchi(Ulanish* db, std::string jadval) : db_(db), jadval_(std::move(jadval)) {}

    SorovQuruvchi& tanlash(const std::string& ustunlar) {
        tanlovlar_ = ustunlar;
        return *this;
    }

    // Parametrli shart (Masalan: qayerda("yosh > ? AND ism = ?", 18, "Sardor"))
    template<typename... Args>
    SorovQuruvchi& qayerda(const std::string& shart, Args&&... args) {
        if (!shart_.empty()) shart_ += " AND ";
        shart_ += "(" + shart + ")";
        (shartParametrlari_.push_back(std::forward<Args>(args)), ...);
        return *this;
    }

    SorovQuruvchi& tartiblash(const std::string& tartib) { tartib_ = tartib; return *this; }
    SorovQuruvchi& chegara(int l) { limit_ = l; return *this; }
    SorovQuruvchi& otkazish(int o) { offset_ = o; return *this; }

    // O'qish operatsiyalari
    Asinxron::Vazifa<Xavfsizlik::Natija<std::vector<Qator>, std::string>> olish() {
        std::string sql = "SELECT " + tanlovlar_ + " FROM " + jadval_;
        if (!shart_.empty()) sql += " WHERE " + shart_;
        if (!tartib_.empty()) sql += " ORDER BY " + tartib_;
        if (limit_ >= 0) sql += " LIMIT " + std::to_string(limit_);
        if (offset_ >= 0) sql += " OFFSET " + std::to_string(offset_);

        co_return co_await db_->bajarishDinamik(sql, shartParametrlari_);
    }

    Asinxron::Vazifa<Xavfsizlik::Natija<Qator, std::string>> birinchi() {
        limit_ = 1;
        auto natija = co_await olish();
        if (!natija) co_return Xavfsizlik::xato(natija.error());
        if (natija->empty()) co_return Xavfsizlik::xato(std::string("Ma'lumot topilmadi"));
        co_return Xavfsizlik::muvaffaqiyat(natija->front());
    }

    // Kiritish va Yangilash (Yozish operatsiyalari)
    Asinxron::Vazifa<Xavfsizlik::Natija<int, std::string>> kiritish(const std::unordered_map<std::string, Qiymat>& malumotlar) {
        std::string sql = "INSERT INTO " + jadval_ + " (";
        std::string values = "VALUES (";
        std::vector<Qiymat> params;

        bool first = true;
        for (const auto& [kalit, qiymat] : malumotlar) {
            if (!first) { sql += ", "; values += ", "; }
            sql += "\"" + kalit + "\""; // SQLi himoyasi
            values += "?";
            params.push_back(qiymat);
            first = false;
        }
        sql += ") " + values + ")";

        auto ijro = co_await db_->ijroDinamik(sql, params);
        if (!ijro) co_return Xavfsizlik::xato(ijro.error());
        co_return Xavfsizlik::muvaffaqiyat(db_->oxirgiKiritilganId());
    }

    Asinxron::Vazifa<Xavfsizlik::Natija<int, std::string>> yangilash(const std::unordered_map<std::string, Qiymat>& malumotlar) {
        if (shart_.empty()) {
            co_return Xavfsizlik::xato(std::string("XAVF: Barcha qatorlarni birdaniga yangilash taqiqlangan! qayerda() foydalaning."));
        }

        std::string sql = "UPDATE " + jadval_ + " SET ";
        std::vector<Qiymat> params;

        bool first = true;
        for (const auto& [kalit, qiymat] : malumotlar) {
            if (!first) sql += ", ";
            sql += "\"" + kalit + "\" = ?"; // SQLi himoyasi
            params.push_back(qiymat);
            first = false;
        }
        sql += " WHERE " + shart_;
        for (const auto& sp : shartParametrlari_) params.push_back(sp);

        co_return co_await db_->ijroDinamik(sql, params);
    }

    Asinxron::Vazifa<Xavfsizlik::Natija<int, std::string>> ochirish() {
        if (shart_.empty()) {
            co_return Xavfsizlik::xato(std::string("XAVF: Barcha qatorlarni o'chirish taqiqlangan! qayerda() foydalaning."));
        }
        std::string sql = "DELETE FROM " + jadval_ + " WHERE " + shart_;
        co_return co_await db_->ijroDinamik(sql, shartParametrlari_);
    }
};

inline SorovQuruvchi Ulanish::jadval(const std::string& nomi) {
    return SorovQuruvchi(this, nomi);
}

#if __has_include(<mysql/mysql.h>)
namespace MySQL {
    class SorovQuruvchi;

    class Ulanish {
    private:
        struct MySQLDeleter {
            void operator()(MYSQL* db) const {
                if (db) mysql_close(db);
            }
        };
        std::shared_ptr<MYSQL> db_;

        explicit Ulanish(MYSQL* db) : db_(db, MySQLDeleter{}) {}

        std::string qochish(const std::string& matn) {
            if (!db_) return matn;
            std::string natija;
            natija.resize(matn.length() * 2 + 1);
            unsigned long len = mysql_real_escape_string(db_.get(), natija.data(), matn.c_str(), matn.length());
            natija.resize(len);
            return natija;
        }

        std::string tayyorlash(const std::string& sql, const std::vector<Qiymat>& parametrlar) {
            std::string natija;
            size_t paramIdx = 0;
            for (char c : sql) {
                if (c == '?' && paramIdx < parametrlar.size()) {
                    std::visit([&](auto&& arg) {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, std::nullptr_t>) {
                            natija += "NULL";
                        } else if constexpr (std::is_same_v<T, int> || std::is_same_v<T, double>) {
                            std::ostringstream ss; ss << arg; natija += ss.str();
                        } else if constexpr (std::is_same_v<T, std::string>) {
                            natija += "'" + qochish(arg) + "'";
                        }
                    }, parametrlar[paramIdx++]);
                } else {
                    natija += c;
                }
            }
            return natija;
        }

    public:
        Ulanish() : db_(nullptr) {}

        static Asinxron::Vazifa<Xavfsizlik::Natija<Ulanish, std::string>> ochish(
            const std::string& host, const std::string& user, const std::string& pass, const std::string& dbname, int port = 3306) {
            MYSQL* db = mysql_init(nullptr);
            if (!db) {
                co_return Xavfsizlik::xato(std::string("MySQL initsializatsiya xatosi"));
            }
            
            if (!mysql_real_connect(db, host.c_str(), user.c_str(), pass.c_str(), dbname.c_str(), port, nullptr, 0)) {
                std::string err = mysql_error(db);
                mysql_close(db);
                co_return Xavfsizlik::xato("Baza ochilmadi: " + err);
            }
            
            mysql_set_character_set(db, "utf8mb4");
            co_return Xavfsizlik::muvaffaqiyat(Ulanish(db));
        }

        Asinxron::Vazifa<Xavfsizlik::Natija<std::vector<Qator>, std::string>> bajarishDinamik(const std::string& sql, const std::vector<Qiymat>& parametrlar) {
            if (!db_) co_return Xavfsizlik::xato(std::string("Baza ulanishi yopiq"));

            std::string tayyorSql = tayyorlash(sql, parametrlar);
            
            if (mysql_query(db_.get(), tayyorSql.c_str())) {
                co_return Xavfsizlik::xato("So'rov bajarilishida xato: " + std::string(mysql_error(db_.get())));
            }

            MYSQL_RES* res = mysql_store_result(db_.get());
            std::vector<Qator> natijalar;
            
            if (res) {
                int num_fields = mysql_num_fields(res);
                MYSQL_FIELD* fields = mysql_fetch_fields(res);
                MYSQL_ROW row;
                
                while ((row = mysql_fetch_row(res))) {
                    Qator qator;
                    for(int i = 0; i < num_fields; i++) {
                        qator[fields[i].name] = row[i] ? row[i] : "";
                    }
                    natijalar.push_back(std::move(qator));
                }
                mysql_free_result(res);
            }
            
            co_return Xavfsizlik::muvaffaqiyat(std::move(natijalar));
        }

        Asinxron::Vazifa<Xavfsizlik::Natija<int, std::string>> ijroDinamik(const std::string& sql, const std::vector<Qiymat>& parametrlar) {
            if (!db_) co_return Xavfsizlik::xato(std::string("Baza ulanishi yopiq"));

            std::string tayyorSql = tayyorlash(sql, parametrlar);
            
            if (mysql_query(db_.get(), tayyorSql.c_str())) {
                co_return Xavfsizlik::xato("Ijro xatosi: " + std::string(mysql_error(db_.get())));
            }

            co_return Xavfsizlik::muvaffaqiyat(static_cast<int>(mysql_affected_rows(db_.get())));
        }

        int oxirgiKiritilganId() const {
            return db_ ? static_cast<int>(mysql_insert_id(db_.get())) : 0;
        }

        inline SorovQuruvchi jadval(const std::string& nomi);
    };

    class JadvalQuruvchi {
        std::string nomi_;
        std::vector<std::string> ustunlar_;
    public:
        explicit JadvalQuruvchi(std::string nomi) : nomi_(std::move(nomi)) {}
        
        JadvalQuruvchi& id() { ustunlar_.push_back("id INT AUTO_INCREMENT PRIMARY KEY"); return *this; }
        JadvalQuruvchi& matn(const std::string& nom) { ustunlar_.push_back("`" + nom + "` TEXT"); return *this; }
        JadvalQuruvchi& butun(const std::string& nom) { ustunlar_.push_back("`" + nom + "` INT"); return *this; }
        JadvalQuruvchi& haqiqiy(const std::string& nom) { ustunlar_.push_back("`" + nom + "` DOUBLE"); return *this; }
        JadvalQuruvchi& mantiqiy(const std::string& nom) { ustunlar_.push_back("`" + nom + "` BOOLEAN"); return *this; }
        
        std::string yigishSql() const {
            std::string sql = "CREATE TABLE IF NOT EXISTS `" + nomi_ + "` (";
            for(size_t i = 0; i < ustunlar_.size(); i++) {
                if (i > 0) sql += ", ";
                sql += ustunlar_[i];
            }
            sql += ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;";
            return sql;
        }
    };

    class SorovQuruvchi {
        Ulanish* db_;
        std::string jadval_;
        std::string tanlovlar_ = "*";
        std::string shart_;
        std::vector<Qiymat> shartParametrlari_;
        std::string tartib_;
        int limit_ = -1;
        int offset_ = -1;

    public:
        SorovQuruvchi(Ulanish* db, std::string jadval) : db_(db), jadval_(std::move(jadval)) {}

        SorovQuruvchi& tanlash(const std::string& ustunlar) { tanlovlar_ = ustunlar; return *this; }

        template<typename... Args>
        SorovQuruvchi& qayerda(const std::string& shart, Args&&... args) {
            if (!shart_.empty()) shart_ += " AND ";
            shart_ += "(" + shart + ")";
            (shartParametrlari_.push_back(std::forward<Args>(args)), ...);
            return *this;
        }

        SorovQuruvchi& tartiblash(const std::string& tartib) { tartib_ = tartib; return *this; }
        SorovQuruvchi& chegara(int l) { limit_ = l; return *this; }
        SorovQuruvchi& otkazish(int o) { offset_ = o; return *this; }

        Asinxron::Vazifa<Xavfsizlik::Natija<std::vector<Qator>, std::string>> olish() {
            std::string sql = "SELECT " + tanlovlar_ + " FROM `" + jadval_ + "`";
            if (!shart_.empty()) sql += " WHERE " + shart_;
            if (!tartib_.empty()) sql += " ORDER BY " + tartib_;
            if (limit_ >= 0) sql += " LIMIT " + std::to_string(limit_);
            if (offset_ >= 0) sql += " OFFSET " + std::to_string(offset_);
            co_return co_await db_->bajarishDinamik(sql, shartParametrlari_);
        }

        Asinxron::Vazifa<Xavfsizlik::Natija<Qator, std::string>> birinchi() {
            limit_ = 1;
            auto natija = co_await olish();
            if (!natija) co_return Xavfsizlik::xato(natija.error());
            if (natija->empty()) co_return Xavfsizlik::xato(std::string("Ma'lumot topilmadi"));
            co_return Xavfsizlik::muvaffaqiyat(natija->front());
        }

        Asinxron::Vazifa<Xavfsizlik::Natija<int, std::string>> kiritish(const std::unordered_map<std::string, Qiymat>& malumotlar) {
            std::string sql = "INSERT INTO `" + jadval_ + "` (";
            std::string values = "VALUES (";
            std::vector<Qiymat> params;
            bool first = true;
            for (const auto& [kalit, qiymat] : malumotlar) {
                if (!first) { sql += ", "; values += ", "; }
                sql += "`" + kalit + "`";
                values += "?";
                params.push_back(qiymat);
                first = false;
            }
            sql += ") " + values + ")";
            auto ijro = co_await db_->ijroDinamik(sql, params);
            if (!ijro) co_return Xavfsizlik::xato(ijro.error());
            co_return Xavfsizlik::muvaffaqiyat(db_->oxirgiKiritilganId());
        }

        Asinxron::Vazifa<Xavfsizlik::Natija<int, std::string>> yangilash(const std::unordered_map<std::string, Qiymat>& malumotlar) {
            if (shart_.empty()) co_return Xavfsizlik::xato(std::string("XAVF: qayerda() yo'q!"));
            std::string sql = "UPDATE `" + jadval_ + "` SET ";
            std::vector<Qiymat> params;
            bool first = true;
            for (const auto& [kalit, qiymat] : malumotlar) {
                if (!first) sql += ", ";
                sql += "`" + kalit + "` = ?";
                params.push_back(qiymat);
                first = false;
            }
            sql += " WHERE " + shart_;
            for (const auto& sp : shartParametrlari_) params.push_back(sp);
            co_return co_await db_->ijroDinamik(sql, params);
        }

        Asinxron::Vazifa<Xavfsizlik::Natija<int, std::string>> ochirish() {
            if (shart_.empty()) co_return Xavfsizlik::xato(std::string("XAVF: qayerda() yo'q!"));
            std::string sql = "DELETE FROM `" + jadval_ + "` WHERE " + shart_;
            co_return co_await db_->ijroDinamik(sql, shartParametrlari_);
        }
    };

    inline SorovQuruvchi Ulanish::jadval(const std::string& nomi) {
        return SorovQuruvchi(this, nomi);
    }
} // namespace MySQL
#endif

} // namespace uzpp::Baza
