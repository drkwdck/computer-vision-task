#include "../ref.h"

/*
    Function: Вычисляет квадрат разности между двумя картинками
    Parameters:
        src_image - левый операнд
        tmpl_image - правый операнд
        dst_image - результат, массив, в которой будет записан квадрат разности между src_image и tmpl_image
        need_norm - нужна ли нормировка
*/
void sqdiff(const vx_image src_image, const vx_image tmpl_image, const vx_image dst_image, bool need_norm);

/*
    Function: Вычисляет корреляцию между двумя изображениями
    Parameters:
        src_image - левый операнд
        tmpl_image - правый операнд
        dst_image - результат, массив, в которой будет записана корреляция между src_image и tmpl_image
        need_norm - нужна ли нормировка
*/
void corr(const vx_image src_image, const vx_image tmpl_image, const vx_image dst_image, bool need_norm);

/*
    Function: Вычисляет разницу между двумя картинками
    Parameters:
        src_image - левый операнд
        tmpl_image - правый операнд
        dst_image - результат, массив, в которой будет записана разница между src_image и tmpl_image
        need_norm - нужна ли нормировка
*/
void coef(const vx_image src_image, const vx_image tmpl_image, const vx_image dst_image, bool need_norm);



vx_status ref_MatchTemplate(const vx_image src_image, const vx_image tmpl_image, vx_image dst_image, match_template_method_enum method) {
    const uint32_t src_width = src_image->width;
    const uint32_t src_height = src_image->height;
    const uint32_t tmpl_width = tmpl_image->width;
    const uint32_t tmpl_height = tmpl_image->height;
    const uint32_t dst_width = dst_image->width;
    const uint32_t dst_height = dst_image->height;

    if (src_width < tmpl_width || src_height < tmpl_height || src_width != dst_width || src_height != dst_height) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // DRY: Сделать бы здесь фабрику для методов, да в C она как-то не очень выглядит
    if (method == CCORR || method == CCORR_NORMED) {
        corr(src_image, tmpl_image, dst_image, method == CCORR_NORMED);
    }

    if (method == SQDIFF || method == SQDIFF_NORMED) {
        sqdiff(src_image, tmpl_image, dst_image, method == SQDIFF_NORMED);
    }

    if (method == CCOEFF || method == CCOEFF_NORMED) {
        coef(src_image, tmpl_image, dst_image, method == CCOEFF_NORMED);
    }
}

void sqdiff(const vx_image src_image, const vx_image tmpl_image, const vx_image dst_image, bool need_norm) {
    const uint32_t src_width = src_image->width;
    const uint32_t src_height = src_image->height;
    const uint32_t tmpl_width = tmpl_image->width;
    const uint32_t tmpl_height = tmpl_image->height;
    const uint32_t dst_width = dst_image->width;
    const uint32_t dst_height = dst_image->height;

    const uint8_t* src_data = src_image->data;
    const uint8_t* tmpl_data = tmpl_image->data;
    uint8_t* dst_data = dst_image->data;

    // Так как мы не ищем шаблон по краям картинки, где он уже не влез бы,
    // считаем, что разница между шаблоном и этими областями картинки - максимальна
    for (uint32_t y = 0; y < src_height; ++y) {
        for (uint32_t x = 0; x < src_width; ++x) {
            dst_data[y * src_width + x] = 255;
        }
    }

    for (uint32_t start_y = 0; start_y < src_height - tmpl_height; ++start_y) {
        for (uint32_t start_x = 0; start_x < src_width - tmpl_width; ++start_x) {
            dst_data[start_y * src_width + start_x] = 0;

            // Так как число выйдет за unint8_t, используем временную переменную
            double sum_accumulator = 0;
            for (uint32_t y = 0; y < tmpl_height; ++y) {
                for (uint32_t x = 0; x < tmpl_width; ++x) {
                    sum_accumulator += pow(tmpl_data[y * tmpl_width + x] - src_data[start_y * src_width + start_x + tmpl_width * y + x], 2);
                }
            }

            dst_data[start_y * src_width + start_x] = log(sum_accumulator);
        }
    }

    if (need_norm) {
        for (uint32_t start_y = 0; start_y < src_height - tmpl_height; ++start_y) {
            for (uint32_t start_x = 0; start_x < src_width - tmpl_width; ++start_x) {
                double src_sum = 0;
                double tmpl_sum = 0;

                for (uint32_t y = 0; y < tmpl_height; ++y) {
                    for (uint32_t x = 0; x < tmpl_width; ++x) {
                        src_sum += pow(src_data[start_y * src_width + start_x + src_width * y + x], 2);
                        tmpl_sum += pow(tmpl_data[y * tmpl_width + x], 2);
                    }
                }

                double old_value = (double)dst_data[start_y * src_width + start_x];
                double norm_coef = log(sqrt(src_sum * tmpl_sum));
                dst_data[start_y * src_width + start_x] = old_value / norm_coef * 255 / 5; // мастшабирование на отрезок [0; 255]. Пятерку подобрал (наверное, не очень круто), хорошо подходит
            }
        }
    }
}


void corr(const vx_image src_image, const vx_image tmpl_image, const vx_image dst_image, bool need_norm) {
    const uint32_t src_width = src_image->width;
    const uint32_t src_height = src_image->height;
    const uint32_t tmpl_width = tmpl_image->width;
    const uint32_t tmpl_height = tmpl_image->height;
    const uint32_t dst_width = dst_image->width;
    const uint32_t dst_height = dst_image->height;

    const uint8_t* src_data = src_image->data;
    const uint8_t* tmpl_data = tmpl_image->data;
    uint8_t* dst_data = dst_image->data;

    // Так как мы не ищем шаблон по краям картинки, где он уже не влез бы,
    // считаем, что разница между шаблоном и этими областями картинки - минимальная
    for (uint32_t y = 0; y < src_height; ++y) {
        for (uint32_t x = 0; x < src_width; ++x) {
            dst_data[y * src_width + x] = 0;
        }
    }

    for (uint32_t start_y = 0; start_y < src_height - tmpl_height; ++start_y) {
        for (uint32_t start_x = 0; start_x < src_width - tmpl_width; ++start_x) {
            dst_data[start_y * src_width + start_x] = 0;

            // Так как число выйдет за unint8_t, используем временную переменную
            uint32_t sum_accumulator = 0;
            for (uint32_t y = 0; y < tmpl_height; ++y) {
                for (uint32_t x = 0; x < tmpl_width; ++x) {
                    sum_accumulator += (tmpl_data[y * tmpl_width + x] + 1) * (src_data[start_y * src_width + start_x + tmpl_width * y + x] + 1);
                }
            }

            dst_data[start_y * src_width + start_x] = sqrt(sum_accumulator / tmpl_width / tmpl_height);
        }
    }

    if (need_norm) {
        for (uint32_t start_y = 0; start_y < src_height - tmpl_height; ++start_y) {
            for (uint32_t start_x = 0; start_x < src_width - tmpl_width; ++start_x) {
                uint32_t src_sum = 0;
                uint32_t tmpl_sum = 0;

                for (uint32_t y = 0; y < tmpl_height; ++y) {
                    for (uint32_t x = 0; x < tmpl_width; ++x) {
                        src_sum += src_data[start_y * src_width + start_x + src_width * y + x];
                        tmpl_sum += tmpl_data[y * tmpl_width + x];
                    }
                }

                double old_value = (double)dst_data[start_y * src_width + start_x];
                double norm_coef = log(sqrt(src_sum * tmpl_sum / tmpl_height / tmpl_width));
                dst_data[start_y * src_width + start_x] = old_value / norm_coef;
            }
        }
    }
}

void coef(const vx_image src_image, const vx_image tmpl_image, const vx_image dst_image, bool need_norm) {
    const uint32_t src_width = src_image->width;
    const uint32_t src_height = src_image->height;
    const uint32_t tmpl_width = tmpl_image->width;
    const uint32_t tmpl_height = tmpl_image->height;
    const uint32_t dst_width = dst_image->width;
    const uint32_t dst_height = dst_image->height;

    const uint8_t* src_data = src_image->data;
    const uint8_t* tmpl_data = tmpl_image->data;
    uint8_t* dst_data = dst_image->data;

    // Параметры алгоритма
    double w = 1;
    double h = tmpl_height * tmpl_width;

    // Так как мы не ищем шаблон по краям картинки, где он уже не влез бы,
    // считаем, что разница между шаблоном и этими областями картинки - минимальная
    for (uint32_t y = 0; y < src_height; ++y) {
        for (uint32_t x = 0; x < src_width; ++x) {
            dst_data[y * src_width + x] = 0;
        }
    }

    // Считаем заранее, так как он не меняется далее
    uint32_t tmp_sum = 0;
    for (uint32_t y = 0; y < tmpl_height; ++y) {
        for (uint32_t x = 0; x < tmpl_width; ++x) {
            tmp_sum += tmpl_data[y * tmpl_width + x];
        }
    }

    for (uint32_t start_y = 0; start_y < src_height - tmpl_height; ++start_y) {
        for (uint32_t start_x = 0; start_x < src_width - tmpl_width; ++start_x) {
            dst_data[start_y * src_width + start_x] = 0;

            // считаем I'
            uint32_t src_sum = 0;
            for (uint32_t y = 0; y < tmpl_height; ++y) {
                for (uint32_t x = 0; x < tmpl_width; ++x) {
                    src_sum += src_data[start_y * src_width + start_x + tmpl_width * y + x];
                }
            }

            // Так как число выйдет за unint8_t, используем временную переменную
            uint32_t sum_accumulator = 0;
            for (uint32_t y = 0; y < tmpl_height; ++y) {
                for (uint32_t x = 0; x < tmpl_width; ++x) {
                    sum_accumulator += (tmpl_data[y * tmpl_width + x]  - (w / h * tmp_sum)) * (src_data[start_y * src_width + start_x + tmpl_width * y + x] - (w / h * src_sum));
                }
            }

            dst_data[start_y * src_width + start_x] = sqrt(sum_accumulator / tmpl_height / tmpl_width);
        }
    }

    if (need_norm) {
        for (uint32_t start_y = 0; start_y < src_height - tmpl_height; ++start_y) {
            for (uint32_t start_x = 0; start_x < src_width - tmpl_width; ++start_x) {
                uint32_t src_sum = 0;
                uint32_t tmpl_sum = 0;

                // считаем I'
                uint32_t src_sum_t = 0;
                for (uint32_t y = 0; y < tmpl_height; ++y) {
                    for (uint32_t x = 0; x < tmpl_width; ++x) {
                        src_sum_t += src_data[start_y * src_width + start_x + tmpl_width * y + x];
                    }
                }

                for (uint32_t y = 0; y < tmpl_height; ++y) {
                    for (uint32_t x = 0; x < tmpl_width; ++x) {
                        src_sum += src_data[start_y * src_width + start_x + src_width * y + x] - w / h * src_sum_t;
                        tmpl_sum += tmpl_data[y * tmpl_width + x] - w / h * tmpl_sum;
                    }
                }

                double old_value = (double)dst_data[start_y * src_width + start_x];
                double norm_coef = log(sqrt(src_sum * tmpl_sum));
                dst_data[start_y * src_width + start_x] = old_value / norm_coef;
            }
        }
    }
}
