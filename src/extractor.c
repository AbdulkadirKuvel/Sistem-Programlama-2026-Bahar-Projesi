#include "tarsau.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

/**
 * @brief Hedef dizin mevcut değilse oluşturur.
 */
StatusCode ensure_directory_exists(const char *dirpath) {
    if (!dirpath || strcmp(dirpath, ".") == 0) {
        return TARSAU_SUCCESS; // Geçerli dizin için işlem yapmaya gerek yok
    }

    // Unix sistemlerinde dizin oluşturma (0755 varsayılan izinle)
    if (mkdir(dirpath, 0755) != 0) {
        // Dizin zaten varsa hata olarak kabul etme
        if (errno != EEXIST) {
            fprintf(stderr, "Hata: '%s' dizini oluşturulamadı!\n", dirpath);
            return ERR_DIR_CREATION_FAILED;
        }
    }
    return TARSAU_SUCCESS;
}

/**
 * @brief Arşiv dosyasının başlığını ayrıştırarak metadata kayıtlarını çıkarır.
 */
StatusCode parse_archive_header(FILE *archive_fp, FileMetadata *metadata_list, int *file_count) {
    char size_buf[HEADER_SIZE_LEN + 1];
    
    // 1. İlk 10 baytı oku (Metadata boyut bilgisi)
    if (fread(size_buf, 1, HEADER_SIZE_LEN, archive_fp) != HEADER_SIZE_LEN) {
        return ERR_CORRUPTED_ARCHIVE;
    }
    size_buf[HEADER_SIZE_LEN] = '\0';
    
    size_t metadata_size = (size_t)strtoul(size_buf, NULL, 10);
    if (metadata_size == 0) {
        return ERR_CORRUPTED_ARCHIVE;
    }

    // 2. Metadata bloğunu RAM'e oku
    char *metadata_block = (char *)malloc(metadata_size + 1);
    if (!metadata_block) {
        return ERR_CORRUPTED_ARCHIVE;
    }

    if (fread(metadata_block, 1, metadata_size, archive_fp) != metadata_size) {
        free(metadata_block);
        return ERR_CORRUPTED_ARCHIVE;
    }
    metadata_block[metadata_size] = '\0';

    // 3. Metadata string'ini parçala (Parsing)
    // Örnek format: |t1,0755,142||t2.txt,0644,85|
    int count = 0;
    char *ptr = metadata_block;

    while (*ptr == '|') {
        ptr++; // '|' karakterini geç
        char *end_rec = strchr(ptr, '|');
        if (!end_rec) break;
        
        *end_rec = '\0'; // Kaydın sonundaki '|' işaretini keserek string'i izole et

        // record şu an "t1,0755,142" formatında
        char *name_token = strtok(ptr, ",");
        char *perm_token = strtok(NULL, ",");
        char *size_token = strtok(NULL, ",");

        if (name_token && perm_token && size_token && count < MAX_INPUT_FILES) {
            // Sadece dosya adını al (path varsa temizle veya aynen koru)
            // Gereksinim gereği orijinal adı güvenle kopyala
            strncpy(metadata_list[count].filename, name_token, MAX_PATH_LEN - 1);
            strncpy(metadata_list[count].permissions, perm_token, 4);
            metadata_list[count].permissions[4] = '\0';
            metadata_list[count].file_size = (size_t)strtoul(size_token, NULL, 10);
            count++;
        } else {
            free(metadata_block);
            return ERR_CORRUPTED_ARCHIVE;
        }

        ptr = end_rec + 1; // Bir sonraki kayda zıpla
    }

    free(metadata_block);
    *file_count = count;
    return TARSAU_SUCCESS;
}

/**
 * @brief Paketleri çözerek dosyaları hedef dizine orijinal izinleriyle yazar.
 */
StatusCode execute_archive_extraction(const AppConfig *config) {
    if (!config) return ERR_INVALID_ARGS;

    FILE *archive_fp = fopen(config->archive_name, "rb");
    if (!archive_fp) {
        fprintf(stderr, "Arşiv dosyası uygunsuz veya bozuk!\n");
        return ERR_FILE_NOT_FOUND;
    }

    FileMetadata metadata_list[MAX_INPUT_FILES];
    int file_count = 0;

    // Header alanını parse et
    if (parse_archive_header(archive_fp, metadata_list, &file_count) != TARSAU_SUCCESS) {
        fprintf(stderr, "Arşiv dosyası uygunsuz veya bozuk!\n");
        fclose(archive_fp);
        return ERR_CORRUPTED_ARCHIVE;
    }

    // Hedef dizinin varlığından emin ol
    if (ensure_directory_exists(config->target_directory) != TARSAU_SUCCESS) {
        fclose(archive_fp);
        return ERR_DIR_CREATION_FAILED;
    }

    unsigned char buffer[BUFSIZ];

    // Her bir dosyayı sırayla arşivden çıkar ve diske yaz
    for (int i = 0; i < file_count; i++) {
        char output_path[MAX_PATH_LEN * 2];
        // Hedef dizin ile dosya adını birleştir (Örn: d1/t1)
        snprintf(output_path, sizeof(output_path), "%s/%s", 
                 config->target_directory, metadata_list[i].filename);

        FILE *out_fp = fopen(output_path, "wb");
        if (!out_fp) {
            fprintf(stderr, "Hata: Çıktı dosyası oluşturulamadı: %s\n", output_path);
            fclose(archive_fp);
            return ERR_PERMISSION_DENIED;
        }

        size_t remaining = metadata_list[i].file_size;
        while (remaining > 0) {
            size_t to_read = (remaining > sizeof(buffer)) ? sizeof(buffer) : remaining;
            size_t bytes_read = fread(buffer, 1, to_read, archive_fp);
            
            if (bytes_read == 0) {
                // Beklenen boyuttan önce dosya bittiyse arşiv bozuktur
                fprintf(stderr, "Arşiv dosyası uygunsuz veya bozuk!\n");
                fclose(out_fp);
                fclose(archive_fp);
                return ERR_CORRUPTED_ARCHIVE;
            }

            fwrite(buffer, 1, bytes_read, out_fp);
            remaining -= bytes_read;
        }

        fclose(out_fp);

        // Orijinal Unix izinlerini (`chmod`) dosyaya geri uygula
        // Oktal string'i (örn: "0755") tamsayıya (base 8) çeviriyoruz
        mode_t mode = (mode_t)strtol(metadata_list[i].permissions, NULL, 8);
        if (chmod(output_path, mode) != 0) {
            fprintf(stderr, "Uyarı: İzinler uygulanamadı: %s\n", output_path);
        }
    }

    fclose(archive_fp);
    printf("%s dizininde dosyalar açıldı.\n", config->target_directory);
    return TARSAU_SUCCESS;
}