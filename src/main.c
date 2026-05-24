#include <tarsau.h>


int main(int argc, char *argv[]) {
    AppConfig config;
    StatusCode status;

    init_default_config(&config);

    status = parse_arguments(argc, argv, &config);
    if (status != TARSAU_SUCCESS) {
        fprintf(stderr, "Hata: Argümanlar ayrıştırılamadı.\n");
        return EXIT_FAILURE;
    }

    if (config.mode == MODE_CREATE_ARCHIVE) {
        status = validate_constraints(&config);
        if (status == TARSAU_SUCCESS) {
            status = execute_archive_creation(&config);
        }
    } else if (config.mode == MODE_EXTRACT_ARCHIVE) {
        status = execute_archive_extraction(&config);
    }

    if (status != TARSAU_SUCCESS) {
        /* TODO: Hata koduna göre özelleştirilmiş loglama operasyonu */
        return EXIT_FAILURE;
    }

    printf("İşlem başarıyla tamamlandı.\n");
    return EXIT_SUCCESS;
}