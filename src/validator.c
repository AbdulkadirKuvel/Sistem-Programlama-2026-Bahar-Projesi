#include <tarsau.h>

/**
 * @brief Dosyanın her bir baytının 0-127 aralığında (ASCII) olup olmadığını kontrol eder.
 */
StatusCode validate_ascii_integrity(const char *filepath)
{
    FILE *fp = fopen(filepath, "rb"); // Bayt seviyesinde okuma için "rb" modu
    if (!fp)
    {
        return ERR_FILE_NOT_FOUND;
    }

    unsigned char buffer[BUFSIZ];
    size_t bytes_read;

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0)
    {
        for (size_t i = 0; i < bytes_read; i++)
        {
            // ASCII karakter seti standardı 0 ile 127 (dahil) arasıdır.
            if (buffer[i] > 127)
            {
                fclose(fp);
                return ERR_INVALID_FORMAT; // Uyumsuz format (non-ASCII)
            }
        }
    }

    fclose(fp);
    return TARSAU_SUCCESS;
}

/**
 * @brief Giriş dosyalarının toplam boyutunu ve adet sınırını denetler.
 */
StatusCode validate_constraints(const AppConfig *config)
{
if (!config) return ERR_INVALID_ARGS;

    size_t total_size = 0;

    for (int i = 0; i < config->input_file_count; i++) {
        const char *filepath = config->input_files[i];
        struct stat st;

        // 1. Dosya var mı ve stat bilgileri okunabiliyor mu?
        if (stat(filepath, &st) != 0) {
            fprintf(stderr, "Hata: '%s' giriş dosyası bulunamadı veya okunamadı!\n", filepath);
            return ERR_FILE_NOT_FOUND;
        }

        // 2. Normal bir dosya mı kontrolü (Dizin vb. olmamalı)
        if (!S_ISREG(st.st_mode)) {
            fprintf(stderr, "Hata: '%s' normal bir dosya değil!\n", filepath);
            return ERR_INVALID_FORMAT;
        }

        // 3. Boyut sınırını kümülatif olarak ekle
        total_size += st.st_size;
        if (total_size > MAX_TOTAL_SIZE_BYTES) {
            fprintf(stderr, "Hata: Giriş dosyalarının toplam boyutu %d MB sınırını geçemez!\n", MAX_TOTAL_SIZE_MB);
            return ERR_SIZE_EXCEEDED;
        }

        // 4. İçerik ASCII mi kontrolü
        StatusCode ascii_status = validate_ascii_integrity(filepath);
        if (ascii_status == ERR_INVALID_FORMAT) {
            fprintf(stderr, "%s giriş dosyasının formatı uyumsuzdur!\n", filepath);
            return ERR_INVALID_FORMAT;
        }
    }

    return TARSAU_SUCCESS;}
