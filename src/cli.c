#include <tarsau.h>

/**
 * @brief Komut satırı argümanlarını doğrular ve AppConfig yapısını doldurur.
 */
StatusCode parse_arguments(int argc, char *argv[], AppConfig *config)
{
    if (argc < 2 || !config)
    {
        return ERR_INVALID_ARGS;
    }

    // İlk parametre işlem modunu belirler (-b veya -a)
    if (strcmp(argv[1], "-b") == 0)
    {
        config->mode = MODE_CREATE_ARCHIVE;

        int i = 2;
        while (i < argc)
        {
            // Eğer -o parametresi geldiyse, bir sonraki argüman arşiv adıdır
            if (strcmp(argv[i], "-o") == 0)
            {
                if (i + 1 >= argc)
                {
                    fprintf(stderr, "Hata: -o parametresinden sonra arşiv adı belirtilmelidir!\n");
                    return ERR_INVALID_ARGS;
                }
                strncpy(config->archive_name, argv[i + 1], MAX_PATH_LEN - 1);
                i += 2; // Hem -o'yu hem de argümanı geç
            }
            else
            {
                // Giriş dosyası ekleme
                if (config->input_file_count >= MAX_INPUT_FILES)
                {
                    fprintf(stderr, "Hata: Giriş dosyası sayısı en fazla %d olabilir.\n", MAX_INPUT_FILES);
                    return ERR_FILE_COUNT_EXCEEDED;
                }
                config->input_files[config->input_file_count] = argv[i];
                config->input_file_count++;
                i++;
            }
        }

        // -b modunda en az bir giriş dosyası olmalıdır
        if (config->input_file_count == 0)
        {
            fprintf(stderr, "Hata: Arşivlenecek giriş dosyası belirtilmedi.\n");
            return ERR_INVALID_ARGS;
        }
    }
    else if (strcmp(argv[1], "-a") == 0)
    {
        config->mode = MODE_EXTRACT_ARCHIVE;

        // -a parametresinden sonra en fazla 2 parametre gelebilir (Arşiv adı ve opsiyonel Dizin)
        // argc; program adını (argv[0]) ve "-a"yı (argv[1]) da saydığı için üst sınır 4'tür. (tarsau -a s1.sau d1)
        if (argc < 3 || argc > 4)
        {
            fprintf(stderr, "Hata: -a parametresi uygunsuz sayıda argüman aldı.\n");
            return ERR_INVALID_ARGS;
        }

        // Arşiv adını al ve kontrol et (*.sau uzantılı olmalı)
        char *archive_param = argv[2];
        size_t len = strlen(archive_param);
        if (len < 5 || strcmp(&archive_param[len - 4], ".sau") != 0)
        {
            fprintf(stderr, "Arşiv dosyası uygunsuz veya bozuk!\n");
            return ERR_INVALID_FORMAT;
        }
        strncpy(config->archive_name, archive_param, MAX_PATH_LEN - 1);

        // Eğer hedef dizin parametresi girilmişse (argc == 4)
        if (argc == 4)
        {
            strncpy(config->target_directory, argv[3], MAX_PATH_LEN - 1);
        }
        else
        {
            // Girilmemişse geçerli dizin (.) varsayılır
            strncpy(config->target_directory, ".", MAX_PATH_LEN - 1);
        }
    }
    else
    {
        fprintf(stderr, "Hata: Geçersiz parametre! Kullanım: -b veya -a\n");
        return ERR_INVALID_ARGS;
    }

    return TARSAU_SUCCESS;
}

/**
 * @brief Varsayılan değerlerin atamasını gerçekleştirir (Örn: a.sau).
 */
void init_default_config(AppConfig *config)
{
    if (!config)
        return;

    config->mode = MODE_NONE;
    memset(config->archive_name, 0, MAX_PATH_LEN);
    memset(config->target_directory, 0, MAX_PATH_LEN);
    config->input_file_count = 0;

    for (int i = 0; i < MAX_INPUT_FILES; i++)
    {
        config->input_files[i] = NULL;
    }

    /* Varsayılan arşiv adı ataması */
    strncpy(config->archive_name, "a.sau", MAX_PATH_LEN - 1);
}