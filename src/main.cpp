// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 whyefkan

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <curl/curl.h>

#include "platform.h"

#include <algorithm>
#include <atomic>
#include <cctype>
#include <cerrno>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

enum StringKey {
    K_MANAGER,
    K_SETTINGS,
    K_ABOUT,
    K_ADD,
    K_LINKS,
    K_PAUSE,
    K_RESUME,
    K_REMOVE,
    K_OPEN,
    K_OPEN_FOLDER,
    K_ST_DOWNLOADING,
    K_ST_PAUSED,
    K_ST_COMPLETED,
    K_ST_ERROR,
    K_ST_CONNECTING,
    K_ST_CANCELED,
    K_NO_DOWNLOADS,
    K_READING_SIZE,
    K_FAILED,
    K_ETA,
    K_FOOTER,
    K_ON,
    K_OFF,
    K_APPEARANCE,
    K_DARK,
    K_LIGHT,
    K_CLASSIC,
    K_DOWNLOADS,
    K_DEFAULT_LINKS,
    K_USED_NEW,
    K_LANGUAGE,
    K_CHROME_EXT,
    K_INSTALL_OPEN,
    K_EXT_NOTE,
    K_ABOUT_DESC,
    K_ABOUT_TECH,
    K_F1,
    K_F2,
    K_F3,
    K_F4,
    K_NO_TELEMETRY,
    K_CLOSE,
    K_COUNT
};

enum AppLanguage {
    LANG_EN = 0,
    LANG_TR = 1,
    LANG_ES = 2,
    LANG_DE = 3,
    LANG_FR = 4,
    LANG_IT = 5,
    LANG_PT = 6,
    LANG_RU = 7,
    LANG_JA = 8,
    LANG_KO = 9,
    LANG_ZH = 10,
    LANG_COUNT = 11
};

static const char* kLanguageNames[LANG_COUNT] = {
    "English", "Türkçe", "Español", "Deutsch", "Français", "Italiano",
    "Português", "Русский", "日本語", "한국어", "中文",
};

static const char* kStrings[LANG_COUNT][K_COUNT] = {
    {
        "download manager", "Settings", "About", "Add", "%d links",
        "Pause", "Resume", "Remove", "Open", "Open folder",
        "DOWNLOADING", "PAUSED", "COMPLETED", "ERROR", "CONNECTING", "CANCELED",
        "No downloads yet. Paste a link and press Add.",
        "reading file size...", "download failed", "%s left",
        "RAM %.1f MB  |  active %d  |  links %d  |  bridge %s", "on", "off",
        "Appearance", "Dark", "Light", "Classic",
        "Downloads", "Default links", "Used for new downloads.",
        "Language", "Chrome extension", "Install / Open Chrome Extensions",
        "Chrome does not allow silent installs. Enable Developer mode, click "
        "'Load unpacked' and select the extension folder above.",
        "Lightweight multi-connection download manager.",
        "C++17  -  Dear ImGui  -  GLFW  -  libcurl",
        "Segmented multi-connection downloads", "Persistent resume across restarts",
        "Automatic reconnect on stalled links", "Chrome extension bridge",
        "Built locally. No telemetry, no accounts.", "Close",
    },
    {
        "indirme yöneticisi", "Ayarlar", "Hakkında", "Ekle", "%d bağlantı",
        "Duraklat", "Devam et", "Kaldır", "Aç", "Klasörde aç",
        "İNDİRİLİYOR", "DURAKLATILDI", "TAMAMLANDI", "HATA", "BAĞLANIYOR", "İPTAL",
        "Henüz indirme yok. Bir bağlantı yapıştırıp Ekle'ye bas.",
        "dosya boyutu okunuyor...", "indirme başarısız", "%s kaldı",
        "RAM %.1f MB  |  aktif %d  |  bağlantı %d  |  köprü %s", "açık", "kapalı",
        "Görünüm", "Koyu", "Açık", "Klasik",
        "İndirmeler", "Varsayılan bağlantı", "Yeni indirmeler için kullanılır.",
        "Dil", "Chrome eklentisi", "Chrome Eklentilerini Kur / Aç",
        "Chrome sessiz kuruluma izin vermez. Geliştirici modunu aç, "
        "'Paketlenmemiş öğe yükle'ye tıkla ve yukarıdaki eklenti klasörünü seç.",
        "Hafif, çok bağlantılı indirme yöneticisi.",
        "C++17  -  Dear ImGui  -  GLFW  -  libcurl",
        "Parçalı / çok bağlantılı indirme", "Yeniden başlatmalar arası kalıcı devam",
        "Takılan bağlantılarda otomatik yeniden bağlanma", "Chrome eklentisi köprüsü",
        "Yerel çalışır. Telemetri ve hesap yok.", "Kapat",
    },
    {
        "gestor de descargas", "Ajustes", "Acerca de", "Añadir", "%d conexiones",
        "Pausar", "Reanudar", "Quitar", "Abrir", "Abrir carpeta",
        "DESCARGANDO", "PAUSADO", "COMPLETADO", "ERROR", "CONECTANDO", "CANCELADO",
        "Aún no hay descargas. Pega un enlace y pulsa Añadir.",
        "leyendo tamaño...", "descarga fallida", "quedan %s",
        "RAM %.1f MB  |  activas %d  |  conexiones %d  |  puente %s", "activo", "inactivo",
        "Apariencia", "Oscuro", "Claro", "Clásico",
        "Descargas", "Conexiones por defecto", "Se usa en nuevas descargas.",
        "Idioma", "Extensión de Chrome", "Instalar / Abrir extensiones de Chrome",
        "Chrome no permite instalaciones silenciosas. Activa el modo desarrollador, "
        "pulsa 'Cargar descomprimida' y elige la carpeta de la extensión.",
        "Gestor de descargas ligero y multiconexión.",
        "C++17  -  Dear ImGui  -  GLFW  -  libcurl",
        "Descargas segmentadas multiconexión", "Reanudación persistente entre reinicios",
        "Reconexión automática en enlaces atascados", "Puente de extensión para Chrome",
        "Funciona en local. Sin telemetría ni cuentas.", "Cerrar",
    },
    {
        "Download-Manager", "Einstellungen", "Über", "Hinzufügen", "%d Verbindungen",
        "Pause", "Fortsetzen", "Entfernen", "Öffnen", "Ordner öffnen",
        "WIRD GELADEN", "PAUSIERT", "FERTIG", "FEHLER", "VERBINDE", "ABGEBROCHEN",
        "Noch keine Downloads. Link einfügen und Hinzufügen drücken.",
        "Dateigröße wird gelesen...", "Download fehlgeschlagen", "noch %s",
        "RAM %.1f MB  |  aktiv %d  |  Verbindungen %d  |  Bridge %s", "an", "aus",
        "Darstellung", "Dunkel", "Hell", "Klassisch",
        "Downloads", "Standard-Verbindungen", "Gilt für neue Downloads.",
        "Sprache", "Chrome-Erweiterung", "Chrome-Erweiterungen installieren / öffnen",
        "Chrome erlaubt keine stillen Installationen. Entwicklermodus aktivieren, "
        "'Entpackte Erweiterung laden' und den Erweiterungsordner wählen.",
        "Leichter Multi-Verbindungs-Download-Manager.",
        "C++17  -  Dear ImGui  -  GLFW  -  libcurl",
        "Segmentierte Multi-Verbindungs-Downloads", "Dauerhafte Fortsetzung nach Neustart",
        "Automatische Wiederverbindung bei hängenden Links", "Chrome-Erweiterungs-Bridge",
        "Läuft lokal. Keine Telemetrie, keine Konten.", "Schließen",
    },
    {
        "gestionnaire de téléchargements", "Paramètres", "À propos", "Ajouter", "%d connexions",
        "Pause", "Reprendre", "Retirer", "Ouvrir", "Ouvrir le dossier",
        "TÉLÉCHARGEMENT", "EN PAUSE", "TERMINÉ", "ERREUR", "CONNEXION", "ANNULÉ",
        "Aucun téléchargement. Collez un lien et cliquez sur Ajouter.",
        "lecture de la taille...", "échec du téléchargement", "%s restant",
        "RAM %.1f MB  |  actifs %d  |  connexions %d  |  pont %s", "actif", "inactif",
        "Apparence", "Sombre", "Clair", "Classique",
        "Téléchargements", "Connexions par défaut", "Utilisé pour les nouveaux téléchargements.",
        "Langue", "Extension Chrome", "Installer / Ouvrir les extensions Chrome",
        "Chrome n'autorise pas l'installation silencieuse. Activez le mode développeur, "
        "cliquez sur « Charger l'extension non empaquetée » et choisissez le dossier.",
        "Gestionnaire de téléchargements léger et multi-connexion.",
        "C++17  -  Dear ImGui  -  GLFW  -  libcurl",
        "Téléchargements segmentés multi-connexion", "Reprise persistante après redémarrage",
        "Reconnexion automatique des liens bloqués", "Pont d'extension Chrome",
        "Fonctionne en local. Sans télémétrie ni compte.", "Fermer",
    },
    {
        "gestore download", "Impostazioni", "Informazioni", "Aggiungi", "%d connessioni",
        "Pausa", "Riprendi", "Rimuovi", "Apri", "Apri cartella",
        "SCARICAMENTO", "IN PAUSA", "COMPLETATO", "ERRORE", "CONNESSIONE", "ANNULLATO",
        "Nessun download. Incolla un link e premi Aggiungi.",
        "lettura dimensione...", "download non riuscito", "%s rimanenti",
        "RAM %.1f MB  |  attivi %d  |  connessioni %d  |  bridge %s", "on", "off",
        "Aspetto", "Scuro", "Chiaro", "Classico",
        "Download", "Connessioni predefinite", "Usato per i nuovi download.",
        "Lingua", "Estensione Chrome", "Installa / Apri estensioni Chrome",
        "Chrome non consente installazioni silenziose. Attiva la modalità sviluppatore, "
        "clicca 'Carica estensione non pacchettizzata' e scegli la cartella.",
        "Gestore download leggero multi-connessione.",
        "C++17  -  Dear ImGui  -  GLFW  -  libcurl",
        "Download segmentati multi-connessione", "Ripresa persistente tra riavvii",
        "Riconnessione automatica sui link bloccati", "Bridge estensione Chrome",
        "Funziona in locale. Nessuna telemetria né account.", "Chiudi",
    },
    {
        "gerenciador de downloads", "Configurações", "Sobre", "Adicionar", "%d conexões",
        "Pausar", "Retomar", "Remover", "Abrir", "Abrir pasta",
        "BAIXANDO", "PAUSADO", "CONCLUÍDO", "ERRO", "CONECTANDO", "CANCELADO",
        "Nenhum download. Cole um link e clique em Adicionar.",
        "lendo tamanho...", "falha no download", "%s restantes",
        "RAM %.1f MB  |  ativos %d  |  conexões %d  |  ponte %s", "ligado", "desligado",
        "Aparência", "Escuro", "Claro", "Clássico",
        "Downloads", "Conexões padrão", "Usado em novos downloads.",
        "Idioma", "Extensão do Chrome", "Instalar / Abrir extensões do Chrome",
        "O Chrome não permite instalação silenciosa. Ative o modo desenvolvedor, "
        "clique em 'Carregar sem compactação' e selecione a pasta.",
        "Gerenciador de downloads leve e multiconexão.",
        "C++17  -  Dear ImGui  -  GLFW  -  libcurl",
        "Downloads segmentados multiconexão", "Retomada persistente após reiniciar",
        "Reconexão automática em links travados", "Ponte de extensão do Chrome",
        "Funciona localmente. Sem telemetria nem contas.", "Fechar",
    },
    {
        "менеджер загрузок", "Настройки", "О программе", "Добавить", "%d соединений",
        "Пауза", "Продолжить", "Удалить", "Открыть", "Открыть папку",
        "ЗАГРУЗКА", "ПРИОСТАНОВЛЕНО", "ЗАВЕРШЕНО", "ОШИБКА", "ПОДКЛЮЧЕНИЕ", "ОТМЕНЕНО",
        "Загрузок пока нет. Вставьте ссылку и нажмите «Добавить».",
        "чтение размера...", "ошибка загрузки", "осталось %s",
        "RAM %.1f МБ  |  активных %d  |  соединений %d  |  мост %s", "вкл", "выкл",
        "Внешний вид", "Тёмная", "Светлая", "Классическая",
        "Загрузки", "Соединений по умолчанию", "Для новых загрузок.",
        "Язык", "Расширение Chrome", "Установить / Открыть расширения Chrome",
        "Chrome не разрешает тихую установку. Включите режим разработчика, нажмите "
        "«Загрузить распакованное расширение» и выберите папку.",
        "Лёгкий многопоточный менеджер загрузок.",
        "C++17  -  Dear ImGui  -  GLFW  -  libcurl",
        "Сегментные многопоточные загрузки", "Сохранение прогресса между запусками",
        "Автопереподключение при зависании", "Мост для расширения Chrome",
        "Работает локально. Без телеметрии и аккаунтов.", "Закрыть",
    },
    {
        "ダウンロードマネージャー", "設定", "このアプリについて", "追加", "%d 接続",
        "一時停止", "再開", "削除", "開く", "フォルダーを開く",
        "ダウンロード中", "一時停止中", "完了", "エラー", "接続中", "キャンセル",
        "ダウンロードはありません。リンクを貼り付けて「追加」を押してください。",
        "サイズを取得中...", "ダウンロード失敗", "残り %s",
        "RAM %.1f MB  |  実行中 %d  |  接続 %d  |  ブリッジ %s", "オン", "オフ",
        "外観", "ダーク", "ライト", "クラシック",
        "ダウンロード", "既定の接続数", "新しいダウンロードに適用されます。",
        "言語", "Chrome 拡張機能", "Chrome 拡張機能をインストール / 開く",
        "Chrome はサイレントインストールを許可していません。デベロッパーモードを有効にし、"
        "「パッケージ化されていない拡張機能を読み込む」でフォルダーを選択してください。",
        "軽量なマルチ接続ダウンロードマネージャー。",
        "C++17  -  Dear ImGui  -  GLFW  -  libcurl",
        "分割マルチ接続ダウンロード", "再起動後も続きから再開",
        "停止した接続の自動再接続", "Chrome 拡張機能ブリッジ",
        "ローカルで動作。テレメトリやアカウントなし。", "閉じる",
    },
    {
        "다운로드 관리자", "설정", "정보", "추가", "%d 연결",
        "일시정지", "계속", "제거", "열기", "폴더 열기",
        "다운로드 중", "일시정지됨", "완료", "오류", "연결 중", "취소됨",
        "다운로드가 없습니다. 링크를 붙여넣고 추가를 누르세요.",
        "파일 크기 확인 중...", "다운로드 실패", "%s 남음",
        "RAM %.1f MB  |  활성 %d  |  연결 %d  |  브리지 %s", "켜짐", "꺼짐",
        "모양", "어둡게", "밝게", "클래식",
        "다운로드", "기본 연결 수", "새 다운로드에 적용됩니다.",
        "언어", "Chrome 확장 프로그램", "Chrome 확장 프로그램 설치 / 열기",
        "Chrome은 자동 설치를 허용하지 않습니다. 개발자 모드를 켜고 "
        "'압축해제된 확장 프로그램을 로드'를 눌러 폴더를 선택하세요.",
        "가벼운 멀티 연결 다운로드 관리자.",
        "C++17  -  Dear ImGui  -  GLFW  -  libcurl",
        "분할 멀티 연결 다운로드", "재시작 후에도 이어받기",
        "멈춘 연결 자동 재연결", "Chrome 확장 프로그램 브리지",
        "로컬에서 동작. 추적 및 계정 없음.", "닫기",
    },
    {
        "下载管理器", "设置", "关于", "添加", "%d 个连接",
        "暂停", "继续", "移除", "打开", "打开文件夹",
        "下载中", "已暂停", "已完成", "错误", "连接中", "已取消",
        "暂无下载。粘贴链接并点击“添加”。",
        "正在读取大小...", "下载失败", "剩余 %s",
        "RAM %.1f MB  |  活动 %d  |  连接 %d  |  桥接 %s", "开", "关",
        "外观", "深色", "浅色", "经典",
        "下载", "默认连接数", "用于新下载。",
        "语言", "Chrome 扩展", "安装 / 打开 Chrome 扩展",
        "Chrome 不允许静默安装。请开启开发者模式，点击“加载已解压的扩展程序”"
        "并选择该文件夹。",
        "轻量级多连接下载管理器。",
        "C++17  -  Dear ImGui  -  GLFW  -  libcurl",
        "分段多连接下载", "重启后持久续传",
        "连接卡住时自动重连", "Chrome 扩展桥接",
        "本地运行。无遥测、无账户。", "关闭",
    },
};

enum DownloadStatus {
    STATUS_DOWNLOADING = 0,
    STATUS_PAUSED = 1,
    STATUS_DONE = 2,
    STATUS_ERROR = 3,
    STATUS_CONNECTING = 4,
};

struct SegmentProgress {
    unsigned long long start = 0;
    unsigned long long end = 0;
    std::atomic<unsigned long long> downloaded{0};

    unsigned long long length() const { return end - start + 1; }
};

struct DownloadJob {
    std::string url;
    int connections = 8;

    std::atomic<unsigned long long> total{0};
    std::atomic<unsigned long long> downloaded{0};
    std::atomic<int> status{STATUS_CONNECTING};
    std::atomic<bool> pause{false};
    std::atomic<bool> abort{false};
    std::atomic<bool> ranges{false};
    std::atomic<int> remaining{0};

    std::vector<std::shared_ptr<SegmentProgress>> segments;
    std::string etag;

    std::mutex meta_mutex;
    std::string name;
    std::string path;
    std::string state_path;

    std::mutex error_mutex;
    std::string error;

    int fd = -1;

    ~DownloadJob() {
        if (fd >= 0) {
            ent::close_file(fd);
            fd = -1;
        }
    }
};

struct DownloadItem {
    std::shared_ptr<DownloadJob> job;
    unsigned long long last_bytes = 0;
    double speed = 0.0;
};

struct IncomingRequest {
    std::string url;
    std::string filename;
};

static std::vector<DownloadItem> g_downloads;
static int g_connections = 8;

static std::mutex g_incoming_mutex;
static std::vector<IncomingRequest> g_incoming;
static std::atomic<bool> g_bridge_ready{false};
static const int kBridgePort = 8765;
static GLFWwindow* g_window = nullptr;

static int g_theme = 0;
static int g_language = LANG_EN;
static bool g_show_settings = false;
static bool g_show_about = false;
static ImFont* g_font_intl = nullptr;

static const char* tr(StringKey key) {
    const char* value = kStrings[g_language][key];
    return value != nullptr ? value : kStrings[LANG_EN][key];
}

#ifndef ENTDM_EXTENSION_DIR
#define ENTDM_EXTENSION_DIR "extension"
#endif

static std::string config_directory() {
    return ent::config_directory();
}

static std::string config_path() {
    return (fs::path(config_directory()) / "config").string();
}

static void load_config() {
    std::ifstream in(config_path());
    std::string line;
    while (std::getline(in, line)) {
        if (line.rfind("theme=", 0) == 0) {
            g_theme = std::atoi(line.c_str() + 6);
            if (g_theme < 0 || g_theme > 2) {
                g_theme = 0;
            }
        } else if (line.rfind("links=", 0) == 0) {
            const int links = std::atoi(line.c_str() + 6);
            g_connections = std::clamp(links, 1, 32);
        } else if (line.rfind("lang=", 0) == 0) {
            g_language = std::atoi(line.c_str() + 5);
            if (g_language < 0 || g_language >= LANG_COUNT) {
                g_language = LANG_EN;
            }
        }
    }
}

static void save_config() {
    std::ofstream out(config_path(), std::ios::trunc);
    if (!out) {
        return;
    }
    out << "theme=" << g_theme << "\n";
    out << "links=" << g_connections << "\n";
    out << "lang=" << g_language << "\n";
}

static void apply_theme(int theme) {
    if (theme == 1) {
        ImGui::StyleColorsLight();
    } else if (theme == 2) {
        ImGui::StyleColorsClassic();
    } else {
        ImGui::StyleColorsDark();
    }
}

static void open_chrome_extension_helper() {
    const std::string chrome = ent::find_chrome();
    if (!chrome.empty()) {
        ent::launch_program(chrome, "chrome://extensions/");
    }
    ent::open_path(ent::extension_directory(ENTDM_EXTENSION_DIR));
}

static std::string fmt_bytes(unsigned long long bytes) {
    static const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    double value = static_cast<double>(bytes);
    int unit = 0;
    while (value >= 1024.0 && unit < 4) {
        value /= 1024.0;
        ++unit;
    }
    char buffer[64];
    if (unit == 0) {
        std::snprintf(buffer, sizeof(buffer), "%llu %s", bytes, units[unit]);
    } else {
        std::snprintf(buffer, sizeof(buffer), "%.1f %s", value, units[unit]);
    }
    return std::string(buffer);
}

static std::string fmt_duration(unsigned long long seconds) {
    char buffer[64];
    if (seconds >= 3600) {
        std::snprintf(buffer, sizeof(buffer), "%lluh %02llum",
                      seconds / 3600, (seconds % 3600) / 60);
    } else if (seconds >= 60) {
        std::snprintf(buffer, sizeof(buffer), "%llum %02llus",
                      seconds / 60, seconds % 60);
    } else {
        std::snprintf(buffer, sizeof(buffer), "%llus", seconds);
    }
    return std::string(buffer);
}

static double current_rss_mb() {
    return ent::process_rss_mb();
}

static std::string to_lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

static std::string trim(const std::string& value) {
    const std::size_t first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return "";
    }
    const std::size_t last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

static int hex_value(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static std::string percent_decode(const std::string& input) {
    std::string out;
    out.reserve(input.size());
    for (std::size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '%' && i + 2 < input.size()) {
            const int high = hex_value(input[i + 1]);
            const int low = hex_value(input[i + 2]);
            if (high >= 0 && low >= 0) {
                out.push_back(static_cast<char>((high << 4) | low));
                i += 2;
                continue;
            }
        }
        out.push_back(input[i]);
    }
    return out;
}

static std::string basename_from_url(const std::string& url) {
    std::string s = url;
    const std::size_t query = s.find_first_of("?#");
    if (query != std::string::npos) {
        s.erase(query);
    }
    const std::size_t slash = s.find_last_of('/');
    if (slash != std::string::npos) {
        s = s.substr(slash + 1);
    }
    return percent_decode(s);
}

static std::string sanitize_filename(std::string name) {
    const std::size_t slash = name.find_last_of("/\\");
    if (slash != std::string::npos) {
        name = name.substr(slash + 1);
    }
    for (char& c : name) {
        const unsigned char uc = static_cast<unsigned char>(c);
        if (uc < 0x20 || c == '/' || c == '\\') {
            c = '_';
        }
    }
    if (name.empty() || name == "." || name == "..") {
        name = "download.bin";
    }
    if (name.size() > 200) {
        name = name.substr(name.size() - 200);
    }
    return name;
}

static fs::path downloads_directory() {
    return fs::path(ent::downloads_directory());
}

static fs::path choose_output_path(const std::string& filename) {
    const fs::path dir = downloads_directory();
    fs::path candidate = dir / filename;
    if (!fs::exists(candidate)) {
        return candidate;
    }
    const std::string stem = candidate.stem().string();
    const std::string ext = candidate.extension().string();
    for (int i = 1; i < 10000; ++i) {
        fs::path next = dir / (stem + " (" + std::to_string(i) + ")" + ext);
        if (!fs::exists(next)) {
            return next;
        }
    }
    return candidate;
}

static std::string state_directory() {
    return ent::data_directory();
}

static std::string state_path_for_url(const std::string& url) {
    unsigned long long hash = 1469598103934665603ull;
    for (unsigned char c : url) {
        hash ^= static_cast<unsigned long long>(c);
        hash *= 1099511628211ull;
    }
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%016llx.state", hash);
    return (fs::path(state_directory()) / buffer).string();
}

struct SavedSegment {
    unsigned long long start = 0;
    unsigned long long end = 0;
    unsigned long long downloaded = 0;
};

struct SavedState {
    bool ok = false;
    bool ranges = false;
    unsigned long long total = 0;
    std::string url;
    std::string etag;
    std::string path;
    std::string name;
    std::vector<SavedSegment> segments;
};

static bool saved_coverage_ok(const SavedState& state) {
    if (state.segments.empty() || state.total == 0) {
        return false;
    }
    unsigned long long cursor = 0;
    for (const SavedSegment& segment : state.segments) {
        if (segment.start != cursor || segment.end < segment.start) {
            return false;
        }
        const unsigned long long length = segment.end - segment.start + 1;
        if (segment.downloaded > length) {
            return false;
        }
        cursor = segment.end + 1;
    }
    return cursor == state.total;
}

static SavedState load_state(const std::string& file) {
    SavedState state;
    std::ifstream in(file);
    if (!in) {
        return state;
    }

    bool have_url = false;
    bool have_total = false;
    std::string line;
    while (std::getline(in, line)) {
        if (line.rfind("url=", 0) == 0) {
            state.url = line.substr(4);
            have_url = true;
        } else if (line.rfind("total=", 0) == 0) {
            state.total = std::strtoull(line.c_str() + 6, nullptr, 10);
            have_total = true;
        } else if (line.rfind("ranges=", 0) == 0) {
            state.ranges = line.substr(7) == "1";
        } else if (line.rfind("etag=", 0) == 0) {
            state.etag = line.substr(5);
        } else if (line.rfind("path=", 0) == 0) {
            state.path = line.substr(5);
        } else if (line.rfind("name=", 0) == 0) {
            state.name = line.substr(5);
        } else if (line.rfind("seg=", 0) == 0) {
            unsigned long long start = 0;
            unsigned long long end = 0;
            unsigned long long downloaded = 0;
            if (std::sscanf(line.c_str() + 4, "%llu,%llu,%llu", &start, &end, &downloaded) == 3) {
                state.segments.push_back(SavedSegment{start, end, downloaded});
            }
        }
    }

    state.ok = have_url && have_total && saved_coverage_ok(state);
    return state;
}

static void write_state(const std::string& file, const std::shared_ptr<DownloadJob>& job) {
    std::string name;
    std::string path;
    {
        std::lock_guard<std::mutex> lock(job->meta_mutex);
        name = job->name;
        path = job->path;
    }

    std::string data;
    data += "version=1\n";
    data += "url=" + job->url + "\n";
    data += "total=" + std::to_string(job->total.load()) + "\n";
    data += "ranges=" + std::string(job->ranges.load() ? "1" : "0") + "\n";
    data += "etag=" + job->etag + "\n";
    data += "path=" + path + "\n";
    data += "name=" + name + "\n";
    for (const auto& segment : job->segments) {
        data += "seg=" + std::to_string(segment->start) + "," + std::to_string(segment->end) + "," +
                std::to_string(segment->downloaded.load()) + "\n";
    }

    const std::string temporary = file + ".tmp";
    std::ofstream out(temporary, std::ios::binary | std::ios::trunc);
    if (!out) {
        return;
    }
    out.write(data.data(), static_cast<std::streamsize>(data.size()));
    out.close();
    std::error_code ec;
    fs::rename(temporary, file, ec);
}

static void state_saver(std::shared_ptr<DownloadJob> job, std::string file) {
    while (true) {
        const int status = job->status.load();
        if (job->abort.load() || status == STATUS_DONE) {
            std::error_code ec;
            fs::remove(file, ec);
            return;
        }
        write_state(file, job);
        if (status == STATUS_ERROR) {
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

static void open_file(const std::string& path) {
    if (path.empty()) {
        return;
    }
    ent::open_path(path);
}

static void open_folder(const std::string& file_path) {
    if (file_path.empty()) {
        return;
    }
    fs::path parent = fs::path(file_path).parent_path();
    if (parent.empty()) {
        parent = fs::current_path();
    }
    ent::open_path(parent.string());
}

static void open_downloads_folder() {
    ent::open_path(downloads_directory().string());
}

static bool header_value(const std::string& headers, const std::string& name, std::string& out) {
    const std::string lower = to_lower(headers);
    const std::string needle = "\r\n" + name + ":";
    const std::size_t pos = lower.rfind(needle);
    if (pos == std::string::npos) {
        return false;
    }
    const std::size_t start = pos + needle.size();
    const std::size_t end = headers.find("\r\n", start);
    const std::string raw = headers.substr(start, end == std::string::npos ? std::string::npos : end - start);
    out = trim(raw);
    return !out.empty();
}

static std::string parse_content_disposition(const std::string& value) {
    std::size_t pos = 0;
    while (pos < value.size()) {
        std::size_t end = value.find(';', pos);
        if (end == std::string::npos) {
            end = value.size();
        }
        const std::string part = trim(value.substr(pos, end - pos));
        const std::string lower = to_lower(part);
        if (lower.rfind("filename*=", 0) == 0) {
            std::string rest = part.substr(10);
            const std::size_t quote = rest.find_last_of('\'');
            if (quote != std::string::npos) {
                rest = rest.substr(quote + 1);
            }
            rest.erase(std::remove(rest.begin(), rest.end(), '"'), rest.end());
            const std::string decoded = percent_decode(trim(rest));
            if (!decoded.empty()) {
                return decoded;
            }
        }
        if (lower.rfind("filename=", 0) == 0) {
            std::string rest = trim(part.substr(9));
            rest.erase(std::remove(rest.begin(), rest.end(), '"'), rest.end());
            if (!rest.empty()) {
                return rest;
            }
        }
        pos = end + 1;
    }
    return "";
}

struct ProbeResult {
    bool ok = false;
    bool ranges = false;
    unsigned long long total = 0;
    std::string filename;
    std::string etag;
    std::string error;
};

static size_t abort_write_cb(char*, size_t, size_t, void*) {
    return 0;
}

static size_t header_collect_cb(char* buffer, size_t size, size_t nitems, void* userdata) {
    auto* headers = static_cast<std::string*>(userdata);
    headers->append(buffer, size * nitems);
    return size * nitems;
}

static ProbeResult probe_url(const std::string& url) {
    ProbeResult result;
    CURL* curl = curl_easy_init();
    if (curl == nullptr) {
        result.error = "curl init failed";
        return result;
    }

    std::string headers;
    char errbuf[CURL_ERROR_SIZE];
    errbuf[0] = '\0';

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L);
    curl_easy_setopt(curl, CURLOPT_RANGE, "0-0");
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "entDM/0.1");
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 20L);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_collect_cb);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, abort_write_cb);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);

    const CURLcode code = curl_easy_perform(curl);
    long http_status = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_status);
    curl_easy_cleanup(curl);

    if (code != CURLE_OK && code != CURLE_WRITE_ERROR) {
        result.error = errbuf[0] != '\0' ? errbuf : curl_easy_strerror(code);
        return result;
    }

    std::string disposition;
    if (header_value(headers, "content-disposition", disposition)) {
        result.filename = parse_content_disposition(disposition);
    }
    std::string etag;
    if (header_value(headers, "etag", etag)) {
        result.etag = etag;
    }

    if (http_status == 206) {
        result.ranges = true;
        std::string content_range;
        if (header_value(headers, "content-range", content_range)) {
            const std::size_t slash = content_range.find('/');
            if (slash != std::string::npos) {
                result.total = std::strtoull(content_range.c_str() + slash + 1, nullptr, 10);
            }
        }
        if (result.total == 0) {
            result.error = "server did not report file size";
            return result;
        }
        result.ok = true;
        return result;
    }

    if (http_status >= 200 && http_status < 300) {
        result.ranges = false;
        std::string content_length;
        if (header_value(headers, "content-length", content_length)) {
            result.total = std::strtoull(content_length.c_str(), nullptr, 10);
        }
        if (result.total == 0) {
            result.error = "server did not report file size";
            return result;
        }
        result.ok = true;
        return result;
    }

    result.error = "HTTP " + std::to_string(http_status);
    return result;
}

static void set_error(const std::shared_ptr<DownloadJob>& job, const std::string& message) {
    {
        std::lock_guard<std::mutex> lock(job->error_mutex);
        if (job->error.empty()) {
            job->error = message;
        }
    }
    job->abort.store(true);
    job->pause.store(false);
}

struct SegmentCtx {
    std::shared_ptr<DownloadJob> job;
    std::shared_ptr<SegmentProgress> segment;
    int fd = -1;
    unsigned long long* written = nullptr;
    long http_status = 0;
};

static size_t write_cb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* ctx = static_cast<SegmentCtx*>(userdata);
    const size_t bytes = size * nmemb;
    if (bytes == 0) {
        return 0;
    }
    if (ctx->job->abort.load() || ctx->job->pause.load()) {
        return 0;
    }
    if (ctx->job->ranges.load() && ctx->http_status != 206) {
        return 0;
    }

    const unsigned long long offset = ctx->segment->start + *ctx->written;
    std::string error;
    if (!ent::write_at_offset(ctx->fd, ptr, bytes, offset, error)) {
        set_error(ctx->job, error);
        return 0;
    }
    *ctx->written += static_cast<unsigned long long>(bytes);
    ctx->segment->downloaded.store(*ctx->written);
    ctx->job->downloaded.fetch_add(static_cast<unsigned long long>(bytes));
    return bytes;
}

static int xfer_cb(void* userdata, curl_off_t, curl_off_t, curl_off_t, curl_off_t) {
    auto* ctx = static_cast<SegmentCtx*>(userdata);
    if (ctx->job->abort.load() || ctx->job->pause.load()) {
        return 1;
    }
    return 0;
}

static size_t segment_header_cb(char* buffer, size_t size, size_t nitems, void* userdata) {
    auto* ctx = static_cast<SegmentCtx*>(userdata);
    const size_t len = size * nitems;
    if (len >= 5 && std::strncmp(buffer, "HTTP/", 5) == 0) {
        const char* space = std::strchr(buffer, ' ');
        if (space != nullptr) {
            ctx->http_status = std::strtol(space + 1, nullptr, 10);
        }
    }
    return len;
}

static void finish_segment(const std::shared_ptr<DownloadJob>& job) {
    if (job->remaining.fetch_sub(1) != 1) {
        return;
    }

    std::string error;
    {
        std::lock_guard<std::mutex> lock(job->error_mutex);
        error = job->error;
    }
    if (!error.empty()) {
        job->status.store(STATUS_ERROR);
        return;
    }
    if (job->abort.load()) {
        return;
    }
    if (job->downloaded.load() >= job->total.load()) {
        job->status.store(STATUS_DONE);
        return;
    }

    {
        std::lock_guard<std::mutex> lock(job->error_mutex);
        if (job->error.empty()) {
            job->error = "incomplete download";
        }
    }
    job->status.store(STATUS_ERROR);
}

static void segment_worker(
    const std::shared_ptr<DownloadJob>& job,
    const std::shared_ptr<SegmentProgress>& segment) {
    const unsigned long long seg_start = segment->start;
    const unsigned long long seg_end = segment->end;
    const unsigned long long seg_len = segment->length();
    unsigned long long written = segment->downloaded.load();

    SegmentCtx ctx;
    ctx.job = job;
    ctx.segment = segment;
    ctx.fd = job->fd;
    ctx.written = &written;

    int attempts = 0;
    while (written < seg_len) {
        if (job->abort.load()) {
            break;
        }
        while (job->pause.load() && !job->abort.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        if (job->abort.load()) {
            break;
        }

        CURL* curl = curl_easy_init();
        if (curl == nullptr) {
            set_error(job, "curl init failed");
            break;
        }

        ctx.http_status = 0;
        char errbuf[CURL_ERROR_SIZE];
        errbuf[0] = '\0';
        const std::string range =
            std::to_string(seg_start + written) + "-" + std::to_string(seg_end);

        curl_easy_setopt(curl, CURLOPT_URL, job->url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "entDM/0.1");
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 20L);
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1L);
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 15L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ctx);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, xfer_cb);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &ctx);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, segment_header_cb);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &ctx);
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
        if (job->ranges.load()) {
            curl_easy_setopt(curl, CURLOPT_RANGE, range.c_str());
        }

        const unsigned long long written_before = written;
        const CURLcode res = curl_easy_perform(curl);
        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        curl_easy_cleanup(curl);

        if (job->abort.load()) {
            break;
        }
        if (job->pause.load()) {
            attempts = 0;
            continue;
        }
        if (written >= seg_len) {
            break;
        }

        if (written > written_before) {
            attempts = 0;
        }

        const bool status_ok = job->ranges.load() ? (http_code == 206)
                                                  : (http_code == 200 || http_code == 206);
        const bool fatal_http = http_code >= 400 && http_code < 500 &&
                                http_code != 408 && http_code != 429 && http_code != 416;
        if (fatal_http) {
            set_error(job, "HTTP " + std::to_string(http_code));
            break;
        }

        if (!(res == CURLE_OK && status_ok)) {
            ++attempts;
            if (!job->ranges.load() || attempts > 5) {
                if (http_code >= 400) {
                    set_error(job, "HTTP " + std::to_string(http_code));
                } else if (errbuf[0] != '\0') {
                    set_error(job, errbuf);
                } else if (res != CURLE_OK) {
                    set_error(job, curl_easy_strerror(res));
                } else {
                    set_error(job, "download stalled");
                }
                break;
            }
            std::this_thread::sleep_for(std::chrono::seconds(attempts));
            continue;
        }

        if (!job->ranges.load()) {
            set_error(job, "connection closed before completion");
            break;
        }

        ++attempts;
        if (attempts > 5) {
            set_error(job, "download stalled");
            break;
        }
        std::this_thread::sleep_for(std::chrono::seconds(attempts));
    }

    finish_segment(job);
}

static void start_job(const std::shared_ptr<DownloadJob>& job) {
    std::thread([job]() {
        if (job->abort.load()) {
            return;
        }
        const ProbeResult probe = probe_url(job->url);
        if (!probe.ok) {
            set_error(job, probe.error.empty() ? "could not read file size" : probe.error);
            job->status.store(STATUS_ERROR);
            return;
        }
        if (job->abort.load()) {
            return;
        }

        std::string name;
        {
            std::lock_guard<std::mutex> lock(job->meta_mutex);
            if (!probe.filename.empty()) {
                job->name = sanitize_filename(probe.filename);
            }
            name = sanitize_filename(job->name.empty() ? basename_from_url(job->url) : job->name);
            job->name = name;
        }

        const std::string state_path = state_path_for_url(job->url);
        job->etag = probe.etag;

        const SavedState saved = load_state(state_path);
        bool resume = probe.ranges && saved.ok && saved.url == job->url &&
                      saved.total == probe.total &&
                      (saved.etag.empty() || probe.etag.empty() || saved.etag == probe.etag) &&
                      !saved.path.empty() && fs::exists(saved.path);

        fs::path output;
        if (resume) {
            output = saved.path;
            if (!saved.name.empty()) {
                std::lock_guard<std::mutex> lock(job->meta_mutex);
                job->name = sanitize_filename(saved.name);
                name = job->name;
            }
        } else {
            output = choose_output_path(name);
        }

        {
            std::lock_guard<std::mutex> lock(job->meta_mutex);
            job->path = output.string();
            job->state_path = state_path;
        }

        if (job->abort.load()) {
            return;
        }

        int fd = -1;
        std::string open_error;
        if (resume) {
            fd = ent::open_output_file(output.string(), false, open_error);
            if (fd < 0) {
                resume = false;
                output = choose_output_path(name);
                std::lock_guard<std::mutex> lock(job->meta_mutex);
                job->path = output.string();
            }
        }
        if (!resume) {
            fd = ent::open_output_file(output.string(), true, open_error);
        }
        if (fd < 0) {
            set_error(job, open_error.empty() ? "cannot create file" : open_error);
            job->status.store(STATUS_ERROR);
            return;
        }
        if (job->abort.load()) {
            ent::close_file(fd);
            std::error_code ec;
            fs::remove(output, ec);
            return;
        }

        job->fd = fd;
        job->total.store(probe.total);
        job->ranges.store(probe.ranges);

        const unsigned long long total = probe.total;
        if (resume) {
            unsigned long long sum = 0;
            for (const SavedSegment& saved_segment : saved.segments) {
                auto segment = std::make_shared<SegmentProgress>();
                segment->start = saved_segment.start;
                segment->end = saved_segment.end;
                const unsigned long long length = saved_segment.end - saved_segment.start + 1;
                const unsigned long long done = std::min(saved_segment.downloaded, length);
                segment->downloaded.store(done);
                sum += done;
                job->segments.push_back(segment);
            }
            job->downloaded.store(sum);
        } else {
            std::vector<std::pair<unsigned long long, unsigned long long>> ranges;
            if (probe.ranges) {
                unsigned long long count = static_cast<unsigned long long>(job->connections);
                if (count > total) {
                    count = total;
                }
                if (count < 1) {
                    count = 1;
                }
                const unsigned long long base = total / count;
                const unsigned long long remainder = total % count;
                unsigned long long start = 0;
                for (unsigned long long i = 0; i < count; ++i) {
                    const unsigned long long len = base + (i < remainder ? 1 : 0);
                    ranges.emplace_back(start, start + len - 1);
                    start += len;
                }
            } else {
                ranges.emplace_back(0, total - 1);
            }

            for (const auto& range : ranges) {
                auto segment = std::make_shared<SegmentProgress>();
                segment->start = range.first;
                segment->end = range.second;
                job->segments.push_back(segment);
            }
            job->downloaded.store(0);
        }

        std::vector<std::shared_ptr<SegmentProgress>> pending;
        for (const auto& segment : job->segments) {
            if (segment->downloaded.load() < segment->length()) {
                pending.push_back(segment);
            }
        }

        job->remaining.store(static_cast<int>(pending.size()));

        if (pending.empty()) {
            job->status.store(STATUS_DONE);
            std::error_code ec;
            fs::remove(state_path, ec);
            return;
        }

        job->status.store(STATUS_DOWNLOADING);
        std::thread(state_saver, job, state_path).detach();

        for (const auto& segment : pending) {
            std::thread([job, segment]() {
                segment_worker(job, segment);
            }).detach();
        }
    }).detach();
}

static void restore_downloads() {
    std::error_code ec;
    const fs::path directory = state_directory();
    if (!fs::exists(directory, ec)) {
        return;
    }

    for (const auto& entry : fs::directory_iterator(directory, ec)) {
        if (ec) {
            break;
        }
        const std::string file = entry.path().string();
        if (file.size() < 6 || file.rfind(".state") != file.size() - 6) {
            continue;
        }
        const SavedState state = load_state(file);
        if (!state.ok) {
            continue;
        }

        auto job = std::make_shared<DownloadJob>();
        job->url = state.url;
        job->connections = g_connections;
        {
            std::lock_guard<std::mutex> lock(job->meta_mutex);
            job->name = state.name.empty() ? sanitize_filename(basename_from_url(state.url))
                                           : sanitize_filename(state.name);
            job->path = state.path;
            job->state_path = file;
        }
        g_downloads.push_back(DownloadItem{job, 0, 0.0});
        start_job(job);
    }
}

static void add_download(const std::string& url, const std::string& filename) {
    for (const DownloadItem& existing : g_downloads) {
        if (existing.job->url != url) {
            continue;
        }
        const int status = existing.job->status.load();
        if (status == STATUS_CONNECTING || status == STATUS_DOWNLOADING ||
            status == STATUS_PAUSED) {
            return;
        }
    }

    auto job = std::make_shared<DownloadJob>();
    job->url = url;
    job->connections = g_connections;

    {
        std::lock_guard<std::mutex> lock(job->meta_mutex);
        job->name = sanitize_filename(filename.empty() ? basename_from_url(url) : filename);
    }

    g_downloads.insert(g_downloads.begin(), DownloadItem{job, 0, 0.0});
    start_job(job);
}

static void update_speeds(float delta_time) {
    for (DownloadItem& item : g_downloads) {
        const unsigned long long done = item.job->downloaded.load();
        const int status = item.job->status.load();
        if (delta_time > 0.0001f && status == STATUS_DOWNLOADING) {
            const double instant = done >= item.last_bytes
                ? static_cast<double>(done - item.last_bytes) / static_cast<double>(delta_time)
                : 0.0;
            item.speed = item.speed * 0.75 + instant * 0.25;
        } else if (status != STATUS_DOWNLOADING) {
            item.speed = 0.0;
        }
        item.last_bytes = done;
    }
}

static std::string json_string_field(const std::string& body, const std::string& key) {
    const std::string needle = "\"" + key + "\"";
    std::size_t pos = body.find(needle);
    if (pos == std::string::npos) {
        return "";
    }
    pos = body.find(':', pos + needle.size());
    if (pos == std::string::npos) {
        return "";
    }
    pos = body.find('"', pos + 1);
    if (pos == std::string::npos) {
        return "";
    }

    std::string out;
    ++pos;
    while (pos < body.size()) {
        const char c = body[pos++];
        if (c == '"') {
            break;
        }
        if (c == '\\' && pos < body.size()) {
            const char escaped = body[pos++];
            switch (escaped) {
                case 'n': out += '\n'; break;
                case 't': out += '\t'; break;
                case 'r': out += '\r'; break;
                default: out += escaped; break;
            }
        } else {
            out += c;
        }
    }
    return out;
}

#ifndef ENTDM_VERSION
#define ENTDM_VERSION "0.1.1"
#endif

static const char* kUpdateManifestUrl =
    "https://raw.githubusercontent.com/whyefkan/entDM/main/latest.json";

enum UpdateState {
    UPDATE_IDLE = 0,
    UPDATE_CHECKING,
    UPDATE_REQUIRED,
    UPDATE_FAILED,
};

static std::atomic<int> g_update_state{UPDATE_IDLE};
static std::mutex g_update_mutex;
static std::string g_update_latest;
static std::string g_update_url;

static std::string update_platform_key() {
#ifdef _WIN32
    return "windows-x86_64";
#else
    return "linux-x86_64";
#endif
}

static bool version_newer(const std::string& candidate, const std::string& current) {
    auto parse = [](const std::string& value) {
        std::vector<int> parts;
        std::string number;
        const std::string input = value + ".";
        for (char c : input) {
            if (c == '.') {
                parts.push_back(number.empty() ? 0 : std::atoi(number.c_str()));
                number.clear();
            } else if (std::isdigit(static_cast<unsigned char>(c))) {
                number.push_back(c);
            }
        }
        return parts;
    };

    const std::vector<int> a = parse(candidate);
    const std::vector<int> b = parse(current);
    const std::size_t count = std::max(a.size(), b.size());
    for (std::size_t i = 0; i < count; ++i) {
        const int left = i < a.size() ? a[i] : 0;
        const int right = i < b.size() ? b[i] : 0;
        if (left != right) {
            return left > right;
        }
    }
    return false;
}

static size_t update_write_cb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* output = static_cast<std::string*>(userdata);
    output->append(ptr, size * nmemb);
    return size * nmemb;
}

static bool http_get_string(const std::string& url, std::string& output) {
    CURL* curl = curl_easy_init();
    if (curl == nullptr) {
        return false;
    }
    char errbuf[CURL_ERROR_SIZE];
    errbuf[0] = '\0';
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "entDM/" ENTDM_VERSION);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, update_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &output);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
    const CURLcode result = curl_easy_perform(curl);
    long status = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    curl_easy_cleanup(curl);
    return result == CURLE_OK && status < 400;
}

static void check_updates_async() {
    int current = g_update_state.load();
    if (current == UPDATE_CHECKING || current == UPDATE_REQUIRED) {
        return;
    }
    if (!g_update_state.compare_exchange_strong(current, UPDATE_CHECKING)) {
        return;
    }

    std::thread([]() {
        std::string body;
        if (!http_get_string(kUpdateManifestUrl, body)) {
            g_update_state.store(UPDATE_FAILED);
            return;
        }

        const std::string latest = json_string_field(body, "version");
        const std::string min_version = json_string_field(body, "min_version");
        const std::string release_page = json_string_field(body, "release_page");
        const std::string download = json_string_field(body, update_platform_key());
        if (latest.empty()) {
            g_update_state.store(UPDATE_FAILED);
            return;
        }

        {
            std::lock_guard<std::mutex> lock(g_update_mutex);
            g_update_latest = latest;
            g_update_url = !download.empty()
                ? download
                : (!release_page.empty()
                       ? release_page
                       : "https://github.com/whyefkan/entDM/releases/latest");
        }

        const bool required = version_newer(latest, ENTDM_VERSION) ||
                              version_newer(min_version, ENTDM_VERSION);
        g_update_state.store(required ? UPDATE_REQUIRED : UPDATE_IDLE);
    }).detach();
}

static void draw_update_modal() {
    if (g_update_state.load() != UPDATE_REQUIRED) {
        return;
    }

    const char* popup_id = "Update required###entdm_update";
    if (!ImGui::IsPopupOpen(popup_id)) {
        ImGui::OpenPopup(popup_id);
    }

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(470.0f, 0.0f), ImGuiCond_Appearing);

    if (ImGui::BeginPopupModal(popup_id, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        std::string latest;
        std::string url;
        {
            std::lock_guard<std::mutex> lock(g_update_mutex);
            latest = g_update_latest;
            url = g_update_url;
        }

        ImGui::TextUnformatted("Update required");
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::TextWrapped(
            "A new version of entDM is available. You must update before you can continue.");
        ImGui::Spacing();
        ImGui::Text("Installed: v%s", ENTDM_VERSION);
        ImGui::Text("Latest:    v%s", latest.c_str());
        ImGui::Spacing();
        if (ImGui::Button("Download update", ImVec2(190.0f, 0.0f))) {
            if (!url.empty()) {
                ent::open_url(url);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Quit entDM", ImVec2(120.0f, 0.0f))) {
            glfwSetWindowShouldClose(g_window, GLFW_TRUE);
        }
        ImGui::EndPopup();
    }
}

static void http_respond(ent::socket_handle client, int code, const std::string& body) {
    const char* reason = (code == 200) ? "OK" : "Bad Request";
    char header[512];
    const int header_len = std::snprintf(
        header, sizeof(header),
        "HTTP/1.1 %d %s\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n\r\n",
        code, reason, body.size());
    if (header_len > 0) {
        ent::socket_write(client, header, header_len);
    }
    if (!body.empty()) {
        ent::socket_write(client, body.data(), static_cast<int>(body.size()));
    }
}

static void http_handle(ent::socket_handle client) {
    std::string request;
    char buffer[4096];
    std::size_t header_end = std::string::npos;

    while (header_end == std::string::npos) {
        const long received = ent::socket_read(client, buffer, static_cast<int>(sizeof(buffer)));
        if (received <= 0) {
            return;
        }
        request.append(buffer, static_cast<std::size_t>(received));
        header_end = request.find("\r\n\r\n");
        if (request.size() > 1024 * 1024) {
            return;
        }
    }

    const std::size_t line_end = request.find("\r\n");
    const std::string request_line = request.substr(0, line_end);
    const bool is_post = request_line.compare(0, 5, "POST ") == 0;
    const bool is_options = request_line.compare(0, 8, "OPTIONS ") == 0;

    std::size_t content_length = 0;
    {
        const std::string head = request.substr(0, header_end);
        const std::string lower = to_lower(head);
        const std::size_t cl = lower.find("content-length:");
        if (cl != std::string::npos) {
            content_length = static_cast<std::size_t>(
                std::strtoul(lower.c_str() + cl + 15, nullptr, 10));
        }
    }

    std::string body = request.substr(header_end + 4);
    while (body.size() < content_length) {
        const long received = ent::socket_read(client, buffer, static_cast<int>(sizeof(buffer)));
        if (received <= 0) {
            break;
        }
        body.append(buffer, static_cast<std::size_t>(received));
    }

    if (is_options) {
        http_respond(client, 200, "{\"ok\":true}");
        return;
    }

    if (request_line.find("GET /ping") == 0) {
        http_respond(client, 200, "{\"app\":\"entDM\",\"ok\":true}");
        return;
    }

    if (is_post && request_line.find("POST /add") == 0) {
        const std::string url = json_string_field(body, "url");
        const std::string filename = json_string_field(body, "filename");
        const bool valid = url.rfind("http://", 0) == 0 ||
                           url.rfind("https://", 0) == 0 ||
                           url.rfind("ftp://", 0) == 0;
        if (!valid) {
            http_respond(client, 400, "{\"ok\":false,\"error\":\"invalid url\"}");
            return;
        }
        {
            std::lock_guard<std::mutex> lock(g_incoming_mutex);
            g_incoming.push_back(IncomingRequest{url, filename});
        }
        http_respond(client, 200, "{\"ok\":true}");
        return;
    }

    http_respond(client, 400, "{\"ok\":false,\"error\":\"unknown route\"}");
}

static void http_server_loop() {
    std::string error;
    const ent::socket_handle server = ent::socket_listen_loopback(kBridgePort, error);
    if (server == ent::invalid_socket) {
        return;
    }

    g_bridge_ready.store(true);
    while (true) {
        const ent::socket_handle client = ent::socket_accept(server);
        if (client == ent::invalid_socket) {
            continue;
        }
        http_handle(client);
        ent::socket_close(client);
    }
}

static void draw_download_card(std::size_t index, int& remove_index) {
    DownloadItem& item = g_downloads[index];
    DownloadJob& job = *item.job;

    std::string name;
    {
        std::lock_guard<std::mutex> lock(job.meta_mutex);
        name = job.name;
    }

    const unsigned long long total = job.total.load();
    const unsigned long long done = job.downloaded.load();
    const int status = job.status.load();
    const float progress = total > 0
        ? std::min(1.0f, static_cast<float>(static_cast<double>(done) / static_cast<double>(total)))
        : 0.0f;

    ImGui::PushID(static_cast<int>(index));
    ImGui::BeginChild("##card", ImVec2(0, 92), true);

    ImGui::TextUnformatted(name.c_str());
    ImGui::SameLine();

    const char* status_text = tr(K_ST_CONNECTING);
    ImVec4 status_color(0.65f, 0.68f, 0.75f, 1.0f);
    if (status == STATUS_DOWNLOADING) {
        status_text = tr(K_ST_DOWNLOADING);
        status_color = ImVec4(0.30f, 0.75f, 0.95f, 1.0f);
    } else if (status == STATUS_PAUSED) {
        status_text = tr(K_ST_PAUSED);
        status_color = ImVec4(0.95f, 0.72f, 0.20f, 1.0f);
    } else if (status == STATUS_DONE) {
        status_text = tr(K_ST_COMPLETED);
        status_color = ImVec4(0.35f, 0.85f, 0.45f, 1.0f);
    } else if (status == STATUS_ERROR) {
        status_text = tr(K_ST_ERROR);
        status_color = ImVec4(0.95f, 0.35f, 0.35f, 1.0f);
    }
    ImGui::TextColored(status_color, "[%s]", status_text);

    if (status == STATUS_ERROR) {
        std::string error;
        {
            std::lock_guard<std::mutex> lock(job.error_mutex);
            error = job.error;
        }
        ImGui::TextColored(ImVec4(0.95f, 0.45f, 0.45f, 1.0f), "%s",
                           error.empty() ? tr(K_FAILED) : error.c_str());
    } else if (status == STATUS_CONNECTING) {
        ImGui::TextDisabled("%s", tr(K_READING_SIZE));
    } else {
        char info[256];
        if (status == STATUS_DOWNLOADING && item.speed > 0.0 && total > 0) {
            const unsigned long long eta = static_cast<unsigned long long>(
                static_cast<double>(total - done) / item.speed);
            char eta_text[128];
            std::snprintf(eta_text, sizeof(eta_text), tr(K_ETA), fmt_duration(eta).c_str());
            std::snprintf(info, sizeof(info), "%s / %s   -   %s/s   -   %s",
                          fmt_bytes(done).c_str(), fmt_bytes(total).c_str(),
                          fmt_bytes(static_cast<unsigned long long>(item.speed)).c_str(),
                          eta_text);
        } else {
            std::snprintf(info, sizeof(info), "%s / %s",
                          fmt_bytes(done).c_str(), fmt_bytes(total).c_str());
        }
        ImGui::TextDisabled("%s", info);
    }

    char overlay_text[32];
    std::snprintf(overlay_text, sizeof(overlay_text), "%.1f%%", progress * 100.0f);
    ImGui::ProgressBar(progress, ImVec2(-1, 0), overlay_text);

    if (status == STATUS_DOWNLOADING) {
        if (ImGui::SmallButton(tr(K_PAUSE))) {
            job.pause.store(true);
            job.status.store(STATUS_PAUSED);
        }
    } else if (status == STATUS_PAUSED) {
        if (ImGui::SmallButton(tr(K_RESUME))) {
            job.pause.store(false);
            job.status.store(STATUS_DOWNLOADING);
        }
    } else if (status == STATUS_DONE) {
        std::string path;
        {
            std::lock_guard<std::mutex> lock(job.meta_mutex);
            path = job.path;
        }
        if (ImGui::SmallButton(tr(K_OPEN))) {
            open_file(path);
        }
        ImGui::SameLine();
        if (ImGui::SmallButton(tr(K_OPEN_FOLDER))) {
            open_folder(path);
        }
    } else {
        ImGui::BeginDisabled();
        ImGui::SmallButton(tr(K_ST_CANCELED));
        ImGui::EndDisabled();
    }

    ImGui::SameLine();
    if (ImGui::SmallButton(tr(K_REMOVE))) {
        job.abort.store(true);
        job.pause.store(false);
        std::string path;
        std::string state_path;
        {
            std::lock_guard<std::mutex> lock(job.meta_mutex);
            path = job.path;
            state_path = job.state_path;
        }
        if (!path.empty()) {
            std::error_code ec;
            fs::remove(path, ec);
        }
        if (!state_path.empty()) {
            std::error_code ec;
            fs::remove(state_path, ec);
        }
        remove_index = static_cast<int>(index);
    }

    ImGui::EndChild();
    ImGui::PopID();
}

static void draw_settings_window() {
    if (!g_show_settings) {
        return;
    }

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(
        ImVec2(viewport->GetCenter().x, viewport->GetCenter().y),
        ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(440, 0), ImGuiCond_Appearing);

    const std::string title = std::string(tr(K_SETTINGS)) + "###settings";
    if (ImGui::Begin(title.c_str(), &g_show_settings, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("%s", tr(K_APPEARANCE));
        if (ImGui::RadioButton(tr(K_DARK), g_theme == 0)) {
            g_theme = 0;
            apply_theme(g_theme);
            save_config();
        }
        ImGui::SameLine();
        if (ImGui::RadioButton(tr(K_LIGHT), g_theme == 1)) {
            g_theme = 1;
            apply_theme(g_theme);
            save_config();
        }
        ImGui::SameLine();
        if (ImGui::RadioButton(tr(K_CLASSIC), g_theme == 2)) {
            g_theme = 2;
            apply_theme(g_theme);
            save_config();
        }

        ImGui::Separator();
        ImGui::Text("%s", tr(K_LANGUAGE));
        ImGui::SetNextItemWidth(240.0f);
        if (g_font_intl != nullptr) {
            ImGui::PushFont(g_font_intl);
        }
        if (ImGui::Combo("##language", &g_language, kLanguageNames, LANG_COUNT)) {
            save_config();
        }
        if (g_font_intl != nullptr) {
            ImGui::PopFont();
        }

        ImGui::Separator();
        ImGui::Text("%s", tr(K_DOWNLOADS));
        ImGui::SetNextItemWidth(120.0f);
        if (ImGui::DragInt(tr(K_DEFAULT_LINKS), &g_connections, 0.2f, 1, 32)) {
            save_config();
        }
        ImGui::TextDisabled("%s", tr(K_USED_NEW));
        if (ImGui::Button(tr(K_OPEN_FOLDER))) {
            open_downloads_folder();
        }
        ImGui::SameLine();
        ImGui::TextDisabled("%s", downloads_directory().string().c_str());

        ImGui::Separator();
        ImGui::Text("%s", tr(K_CHROME_EXT));
        ImGui::TextDisabled("%s", ent::extension_directory(ENTDM_EXTENSION_DIR).c_str());
        if (ImGui::Button(tr(K_INSTALL_OPEN))) {
            open_chrome_extension_helper();
        }
        ImGui::TextWrapped("%s", tr(K_EXT_NOTE));
    }
    ImGui::End();
}

static void draw_about_window() {
    if (!g_show_about) {
        return;
    }

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(
        ImVec2(viewport->GetCenter().x, viewport->GetCenter().y),
        ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(430, 0), ImGuiCond_Appearing);

    const std::string title = std::string(tr(K_ABOUT)) + " entDM###about";
    if (ImGui::Begin(title.c_str(), &g_show_about, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("entDM");
        ImGui::SameLine();
        ImGui::TextDisabled("v%s", ENTDM_VERSION);
        ImGui::Separator();
        ImGui::TextWrapped("%s", tr(K_ABOUT_DESC));
        ImGui::Spacing();
        ImGui::BulletText("%s", tr(K_F1));
        ImGui::BulletText("%s", tr(K_F2));
        ImGui::BulletText("%s", tr(K_F3));
        ImGui::BulletText("%s", tr(K_F4));
        ImGui::Spacing();
        ImGui::TextDisabled("%s", tr(K_NO_TELEMETRY));
        if (ImGui::Button(tr(K_CLOSE), ImVec2(-1, 0))) {
            g_show_about = false;
        }
    }
    ImGui::End();
}

static void draw_ui() {
    static double next_update_check = 0.0;
    const double now = ImGui::GetTime();
    if (now >= next_update_check) {
        next_update_check = now + 1800.0;
        check_updates_async();
    }

    const bool use_intl_font = g_language != LANG_EN && g_font_intl != nullptr;
    if (use_intl_font) {
        ImGui::PushFont(g_font_intl);
    }
    {
        std::lock_guard<std::mutex> lock(g_incoming_mutex);
        for (const IncomingRequest& request : g_incoming) {
            add_download(request.url, request.filename);
        }
        g_incoming.clear();
    }

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    const ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::Begin("entDM", nullptr, flags);

    const float window_button = 28.0f;
    const float settings_button = 66.0f;
    const float about_button = 56.0f;
    const float button_spacing = ImGui::GetStyle().ItemSpacing.x;
    const float title_row_height = ImGui::GetFrameHeight();
    const float drag_width = ImGui::GetContentRegionAvail().x
        - (window_button * 2.0f + settings_button + about_button + button_spacing * 4.0f);

    ImGui::InvisibleButton("##title_drag", ImVec2(drag_width, title_row_height));

    static bool dragging = false;
    static int drag_window_x = 0;
    static int drag_window_y = 0;
    static double drag_cursor_x = 0.0;
    static double drag_cursor_y = 0.0;

    if (ImGui::IsItemActivated() && g_window != nullptr) {
        int window_x = 0;
        int window_y = 0;
        double cursor_x = 0.0;
        double cursor_y = 0.0;
        glfwGetWindowPos(g_window, &window_x, &window_y);
        glfwGetCursorPos(g_window, &cursor_x, &cursor_y);
        drag_window_x = window_x;
        drag_window_y = window_y;
        drag_cursor_x = static_cast<double>(window_x) + cursor_x;
        drag_cursor_y = static_cast<double>(window_y) + cursor_y;
        dragging = true;
    }

    if (dragging && g_window != nullptr) {
        if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            int window_x = 0;
            int window_y = 0;
            double cursor_x = 0.0;
            double cursor_y = 0.0;
            glfwGetWindowPos(g_window, &window_x, &window_y);
            glfwGetCursorPos(g_window, &cursor_x, &cursor_y);
            const double global_x = static_cast<double>(window_x) + cursor_x;
            const double global_y = static_cast<double>(window_y) + cursor_y;
            glfwSetWindowPos(g_window,
                             drag_window_x + static_cast<int>(global_x - drag_cursor_x),
                             drag_window_y + static_cast<int>(global_y - drag_cursor_y));
        } else {
            dragging = false;
        }
    }

    ImDrawList* title_draw = ImGui::GetWindowDrawList();
    const ImVec2 title_pos = ImGui::GetItemRectMin();
    const float title_text_y = title_pos.y + (title_row_height - ImGui::GetTextLineHeight()) * 0.5f;
    title_draw->AddText(ImVec2(title_pos.x, title_text_y),
                        ImGui::GetColorU32(ImGuiCol_Text), "entDM");
    const float title_width = ImGui::CalcTextSize("entDM").x;
    title_draw->AddText(ImVec2(title_pos.x + title_width + 8.0f, title_text_y),
                        ImGui::GetColorU32(ImGuiCol_TextDisabled), tr(K_MANAGER));

    ImGui::SameLine();
    if (ImGui::Button(tr(K_SETTINGS), ImVec2(settings_button, title_row_height))) {
        g_show_settings = true;
    }
    ImGui::SameLine();
    if (ImGui::Button(tr(K_ABOUT), ImVec2(about_button, title_row_height))) {
        g_show_about = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("-", ImVec2(window_button, title_row_height)) && g_window != nullptr) {
        glfwIconifyWindow(g_window);
    }
    ImGui::SameLine();
    if (ImGui::Button("X", ImVec2(window_button, title_row_height)) && g_window != nullptr) {
        glfwSetWindowShouldClose(g_window, GLFW_TRUE);
    }

    ImGui::Separator();

    static char url[512] = "";
    const bool can_add = url[0] != '\0';

    const float links_width = 110.0f;
    const float add_width = 80.0f;
    const float row_spacing = ImGui::GetStyle().ItemSpacing.x;
    ImGui::SetNextItemWidth(
        ImGui::GetContentRegionAvail().x - (links_width + add_width + row_spacing * 2.0f));
    const bool submitted = ImGui::InputTextWithHint(
        "##url", "https://example.com/file.zip", url, IM_ARRAYSIZE(url),
        ImGuiInputTextFlags_EnterReturnsTrue);

    ImGui::SameLine();
    ImGui::SetNextItemWidth(links_width);
    ImGui::DragInt("##connections", &g_connections, 0.2f, 1, 32, tr(K_LINKS));

    ImGui::SameLine();
    if (!can_add) {
        ImGui::BeginDisabled();
    }
    const bool add_clicked = ImGui::Button(tr(K_ADD), ImVec2(add_width, 0.0f));
    if (!can_add) {
        ImGui::EndDisabled();
    }
    if ((add_clicked || submitted) && can_add) {
        add_download(url, "");
        url[0] = '\0';
    }

    ImGui::Separator();

    const float footer_height = ImGui::GetFrameHeightWithSpacing() + 4.0f;
    ImGui::BeginChild("##downloads", ImVec2(0, -footer_height), false);

    if (g_downloads.empty()) {
        ImGui::TextDisabled("%s", tr(K_NO_DOWNLOADS));
    }

    int remove_index = -1;
    for (std::size_t i = 0; i < g_downloads.size(); ++i) {
        draw_download_card(i, remove_index);
        ImGui::Spacing();
    }
    if (remove_index >= 0) {
        g_downloads.erase(g_downloads.begin() + remove_index);
    }

    ImGui::EndChild();

    ImGui::Separator();

    int active = 0;
    for (const DownloadItem& item : g_downloads) {
        const int status = item.job->status.load();
        if (status == STATUS_DOWNLOADING || status == STATUS_CONNECTING) {
            ++active;
        }
    }

    static double rss = 0.0;
    static double rss_timer = 0.0;
    rss_timer += ImGui::GetIO().DeltaTime;
    if (rss_timer >= 1.0) {
        rss = current_rss_mb();
        rss_timer = 0.0;
    }

    char footer[256];
    std::snprintf(footer, sizeof(footer), tr(K_FOOTER), rss, active, g_connections,
                  g_bridge_ready.load() ? tr(K_ON) : tr(K_OFF));
    ImGui::TextDisabled("%s", footer);

    draw_update_modal();

    ImGui::End();

    draw_settings_window();
    draw_about_window();

    if (use_intl_font) {
        ImGui::PopFont();
    }
}

int main() {
    ent::platform_init();
    curl_global_init(CURL_GLOBAL_DEFAULT);
    restore_downloads();

    std::thread(http_server_loop).detach();

    if (!glfwInit()) {
        std::fprintf(stderr, "GLFW init failed\n");
        return 1;
    }
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(600, 420, "entDM", nullptr, nullptr);
    if (window == nullptr) {
        std::fprintf(stderr, "Window creation failed\n");
        glfwTerminate();
        return 1;
    }
    g_window = window;

    if (GLFWmonitor* monitor = glfwGetPrimaryMonitor()) {
        if (const GLFWvidmode* mode = glfwGetVideoMode(monitor)) {
            glfwSetWindowPos(window, (mode->width - 600) / 2, (mode->height - 420) / 2);
        }
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    load_config();
    apply_theme(g_theme);

    static ImVector<ImWchar> intl_ranges;
    ImFontAtlas* atlas = io.Fonts;
    ImFont* default_font = atlas->AddFontDefault();
    ImFontGlyphRangesBuilder builder;
    builder.AddRanges(atlas->GetGlyphRangesDefault());
    builder.AddRanges(atlas->GetGlyphRangesCyrillic());
    for (int language = 0; language < LANG_COUNT; ++language) {
        for (int key = 0; key < K_COUNT; ++key) {
            if (kStrings[language][key] != nullptr) {
                builder.AddText(kStrings[language][key]);
            }
        }
    }
    for (int language = 0; language < LANG_COUNT; ++language) {
        builder.AddText(kLanguageNames[language]);
    }
    builder.BuildRanges(&intl_ranges);

    const ent::FontPaths fonts = ent::system_fonts();
    if (!fonts.regular.empty()) {
        g_font_intl =
            atlas->AddFontFromFileTTF(fonts.regular.c_str(), 13.0f, nullptr, intl_ranges.Data);
    }
    if (g_font_intl != nullptr) {
        for (const std::string& cjk : fonts.cjk) {
            ImFontConfig merge_config;
            merge_config.MergeMode = true;
            merge_config.PixelSnapH = true;
            merge_config.FontNo = 0;
            atlas->AddFontFromFileTTF(cjk.c_str(), 13.0f, &merge_config, intl_ranges.Data);
        }
    }
    if (g_font_intl == nullptr) {
        g_font_intl = default_font;
    }

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        update_speeds(io.DeltaTime);
        draw_ui();

        ImGui::Render();
        int display_w = 0;
        int display_h = 0;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.06f, 0.06f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    curl_global_cleanup();
    ent::platform_shutdown();
    return 0;
}
