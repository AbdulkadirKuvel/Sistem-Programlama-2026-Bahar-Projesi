#include <tarsau.h>


/* 3. ARCHIVER MODULE (Serialization & Packaging)                             */

/**
 * @brief Dosyanın Unix izin kodunu alıp 4 haneli oktal string'e dönüştürür.
 */
void get_unix_permissions(const char *filepath, char *out_perm_str) {
    /* TODO: stat yapısı üzerinden S_IRWXU vb. maskelerin okunup formatlanması */
}

/**
 * @brief Metadata içeriğini |ad,izin,boyut| formatında serileştirir.
 */
size_t build_metadata_header(const AppConfig *config, char **out_header_buffer) {
    /* TODO: Dinamik bellek tahsisi ve string operasyonları ile header üretimi */
    return 0;
}

/**
 * @brief Arşivi oluşturur, önce 10 bayt boyutu, ardından metadata ve dosyaları yazar.
 */
StatusCode execute_archive_creation(const AppConfig *config) {
    /* TODO: Metadata ve ardışık veri payload'unun buffering ile diske yazılması */
    return TARSAU_SUCCESS;
}
