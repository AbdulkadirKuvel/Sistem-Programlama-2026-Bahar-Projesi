#include <tarsau.h>

int main(int argc, char *argv[]) {
    AppConfig config;
    StatusCode status;

    // 1. Konfigürasyonu varsayılan değerlerle ilklendir
    init_default_config(&config);

    // 2. CLI Argümanlarını ayrıştır ve ön doğrulamaları yap
    status = parse_arguments(argc, argv, &config);
    if (status != TARSAU_SUCCESS) {
        // Hata mesajları fonksiyonların içinde basıldığı için doğrudan çıkış yapılır
        return EXIT_FAILURE;
    }
    // fprintf(stdout, "Bilgi: operational mode: %d\n", config.mode);

    // 3. Çalışma moduna göre ilgili motoru tetikle
    if (config.mode == MODE_CREATE_ARCHIVE) {
        // Arşivleme öncesi katı kısıtları (Boyut, Adet, ASCII kontrolü) denetle
        status = validate_constraints(&config);
        if (status != TARSAU_SUCCESS) {
            return EXIT_FAILURE;
        }
        
        // Arşivi oluştur
        status = execute_archive_creation(&config);
        if (status != TARSAU_SUCCESS) {
            return EXIT_FAILURE;
        }

    } else if (config.mode == MODE_EXTRACT_ARCHIVE) {
        // Arşivi çöz
        status = execute_archive_extraction(&config);
        if (status != TARSAU_SUCCESS) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}