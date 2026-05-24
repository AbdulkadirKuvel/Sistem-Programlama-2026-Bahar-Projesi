#include <tarsau.h>
#include <sys/stat.h>
#include <libgen.h>

/**
 * @brief Dosyanın Unix izin kodunu alıp 4 haneli oktal string'e dönüştürür.
 */
void get_unix_permissions(const char *filepath, char *out_perm_str)
{
    struct stat st;
    if (stat(filepath, &st) == 0)
    {
        // İzin maskesini filtrele (Kullanıcı, Grup, Diğerleri - Okuma/Yazma/Çalıştırma)
        int perm = st.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
        // 4 haneli oktal string olarak formatla (Örn: 0755)
        snprintf(out_perm_str, 5, "0%o", perm);
    }
    else
    {
        // Hata durumunda güvenli bir varsayılan değer ata
        strncpy(out_perm_str, "0644", 5);
    }
}

/**
 * @brief Metadata içeriğini |ad,izin,boyut| formatında hafızada serileştirir.
 * @return Oluşturulan başlığın (header) toplam karakter boyutunu döner.
 */
size_t build_metadata_header(const AppConfig *config, char **out_header_buffer)
{
    // Tahmini bir başlangıç boyutu belirleyelim (Her dosya için ortalama 100 karakter)
    size_t buffer_size = config->input_file_count * 128;
    char *header_buf = (char *)malloc(buffer_size);
    if (!header_buf)
    {
        return 0;
    }

    header_buf[0] = '\0';
    size_t current_len = 0;

    for (int i = 0; i < config->input_file_count; i++)
    {
        char perm_str[5];
        struct stat st;

        get_unix_permissions(config->input_files[i], perm_str);
        stat(config->input_files[i], &st);

        char path_copy[MAX_PATH_LEN];
        strncpy(path_copy, config->input_files[i], sizeof(path_copy) - 1);
        char *pure_filename = basename(path_copy);

        // Tek bir kayıt şablonu: |t1,0755,142|
        // |dosyaadı,izinler,boyut|
        char record[MAX_PATH_LEN + 32];
        snprintf(record, sizeof(record), "|%s,%s,%ld|",
                 pure_filename, perm_str, (long)st.st_size);

        size_t record_len = strlen(record);

        // Dinamik buffer'ın taşma ihtimaline karşı realloc kontrolü
        if (current_len + record_len >= buffer_size)
        {
            buffer_size *= 2;
            char *new_buf = (char *)realloc(header_buf, buffer_size);
            if (!new_buf)
            {
                free(header_buf);
                return 0;
            }
            header_buf = new_buf;
        }

        strcat(header_buf, record);
        current_len += record_len;
    }

    *out_header_buffer = header_buf;
    return current_len;
}

/**
 * @brief Arşivi oluşturur, önce 10 bayt boyutu, ardından metadata ve dosyaları yazar.
 */
StatusCode execute_archive_creation(const AppConfig *config)
{
    if (!config)
        return ERR_INVALID_ARGS;

    char *metadata_block = NULL;
    size_t metadata_size = build_metadata_header(config, &metadata_block);

    if (metadata_size == 0 || !metadata_block)
    {
        fprintf(stderr, "Hata: Arşiv başlığı oluşturulurken bellek hatası meydana geldi.\n");
        return ERR_CORRUPTED_ARCHIVE;
    }

    // Hedef arşiv dosyasını yazma modunda aç
    FILE *archive_fp = fopen(config->archive_name, "wb");
    if (!archive_fp)
    {
        fprintf(stderr, "Hata: '%s' arşiv dosyası yazılamadı!\n", config->archive_name);
        free(metadata_block);
        return ERR_PERMISSION_DENIED;
    }

    // 1. İLK BÖLÜM: Solu sıfır dolgulu 10 baytlık metadata boyutu yazılıyor
    // %010zu ifadesi tam olarak 10 karakter genişliğinde ve boşlukları '0' ile doldurarak yazar.
    fprintf(archive_fp, "%010zu", metadata_size);

    // 2. İKİNCİ BÖLÜM: Metadata kayıt bloğu yazılıyor
    fwrite(metadata_block, 1, metadata_size, archive_fp);
    free(metadata_block); // RAM'deki geçici metadata bloğunu temizle

    // 3. ÜÇÜNCÜ BÖLÜM: Giriş dosyalarının içerikleri stream edilerek ekleniyor
    unsigned char buffer[BUFSIZ];
    for (int i = 0; i < config->input_file_count; i++)
    {
        FILE *input_fp = fopen(config->input_files[i], "rb");
        if (!input_fp)
        {
            // Validasyondan geçtiği için buraya düşmesi ekstrem bir durumdur (örn. anlık silinme)
            fprintf(stderr, "Hata: Giriş dosyası okunurken hata: %s\n", config->input_files[i]);
            fclose(archive_fp);
            return ERR_FILE_NOT_FOUND;
        }

        size_t bytes_read;
        while ((bytes_read = fread(buffer, 1, sizeof(buffer), input_fp)) > 0)
        {
            fwrite(buffer, 1, bytes_read, archive_fp);
        }

        fclose(input_fp);
    }

    fclose(archive_fp);
    printf("Dosyalar birleştirildi.\n");
    return TARSAU_SUCCESS;
}