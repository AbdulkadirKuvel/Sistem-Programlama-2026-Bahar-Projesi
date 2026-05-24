#ifndef TARSAU_H
#define TARSAU_H

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>


#define MAX_INPUT_FILES 32
#define MAX_PATH_LEN 512
#define HEADER_SIZE_LEN 10
#define MAX_TOTAL_SIZE_MB 200
#define MAX_TOTAL_SIZE_BYTES (MAX_TOTAL_SIZE_MB * 1024 * 1024)

/* Hata Kodları Sınıflandırması */
typedef enum {
    TARSAU_SUCCESS = 0,
    ERR_INVALID_ARGS, // Geçersiz argüman.
    ERR_FILE_NOT_FOUND, // Olmayan dosya.
    ERR_INVALID_FORMAT, // Metin dosyası dışı.
    ERR_SIZE_EXCEEDED, // 200 MB sınırı aşılması.
    ERR_FILE_COUNT_EXCEEDED, // 32 dosya sınırı aşılması.
    ERR_DIR_CREATION_FAILED, // Klasör oluşturma hatası.
    ERR_PERMISSION_DENIED, // İzin yok.
    ERR_CORRUPTED_ARCHIVE // Arşiv bozuk
} StatusCode;

/* Uygulama Modu */
typedef enum {
    MODE_NONE,
    MODE_CREATE_ARCHIVE, /* -b */
    MODE_EXTRACT_ARCHIVE /* -a */
} OperationalMode;

/* Merkezi Konfigürasyon Yapısı */
typedef struct {
    OperationalMode mode;
    char archive_name[MAX_PATH_LEN];
    char target_directory[MAX_PATH_LEN];
    char *input_files[MAX_INPUT_FILES];
    int input_file_count;
} AppConfig;

/* Metadata Dosya Kayıt Yapısı */
typedef struct {
    char filename[MAX_PATH_LEN];
    char permissions[5]; // Unix oktal formatı için (Örn: "0755" + \0)
    size_t file_size;
} FileMetadata;

StatusCode parse_arguments(int argc, char *argv[], AppConfig *config);
void init_default_config(AppConfig *config);

StatusCode validate_ascii_integrity(const char *filepath);
StatusCode validate_constraints(const AppConfig *config);

StatusCode ensure_directory_exists(const char *dirpath);
StatusCode parse_archive_header(FILE *archive_fp, FileMetadata *metadata_list, int *file_count);
StatusCode execute_archive_extraction(const AppConfig *config);

void get_unix_permissions(const char *filepath, char *out_perm_str);
size_t build_metadata_header(const AppConfig *config, char **out_header_buffer);
StatusCode execute_archive_creation(const AppConfig *config);

#endif