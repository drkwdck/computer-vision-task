#include "../ref.h"

/*
    Function: Вычисляет корреляцию между двумя картинками
    Parameters:
        a - левый операнд
        b - правый операнд
        result - результат, массив, в которой будет записана корреляция между a и b
*/
void corr(const vx_image a, const vx_image b, vx_image result);


void norm(const vx_image src_image, const vx_image tmpl_image, vx_image result);


vx_status ref_MatchTemplate(const vx_image src_image, const vx_image tmpl_image, vx_image dst_image, match_template_method_enum method) {
    const uint32_t src_width = src_image->width;
    const uint32_t src_height = src_image->height;
    const uint32_t tmpl_width = tmpl_image->width;
    const uint32_t tmpl_height = tmpl_image->height;
    const uint32_t dst_width = dst_image->width;
    const uint32_t dst_height = dst_image->height;

    if (src_width <  tmpl_width || src_height < tmpl_height || src_width != dst_width || src_height != dst_height) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    const uint8_t* src_data = src_image->data;
    const uint8_t* tmpl_data = tmpl_image->data;
    uint8_t* dst_data = dst_image->data;

    if (method == CCORR || method == CCORR_NORMED) {
        corr(src_image, tmpl_image, dst_image);

        if (method == CCORR_NORMED) {
            norm(src_image, tmpl_image, dst_image);
        }
    }

    if (method == SQDIFF || method == SQDIFF_NORMED) {

        for (uint32_t y = 0; y < src_height; ++y) {
            for (uint32_t x = 0; x < src_width; ++x) {
                dst_data[y * src_width + x] = 255;
            }
        }

        for (uint32_t start_y = 0; start_y < src_height - tmpl_height; ++start_y) {
            for (uint32_t start_x = 0; start_x < src_width - tmpl_width; ++start_x) {
                dst_data[start_y * src_width + start_x] = 0;

                int a = 0;
                for (uint32_t y = 0; y < tmpl_height; ++y) {
                    for (uint32_t x = 0; x < tmpl_width; ++x) {
                        a += abs(tmpl_data[y * tmpl_width + x] - src_data[start_y * src_width + start_x + tmpl_width * y + x]);
                    }
                }
                
                int b = a / tmpl_height / tmpl_width;
                dst_data[start_y * src_width + start_x] = b;
            }
        }

        if (method == SQDIFF_NORMED) {
            //norm(src_image, tmpl_image, dst_image);
        }
    }

    if (method == CCOEFF || method == CCOEFF_NORMED) {
        if (method == CCOEFF_NORMED) {
            
        }
    }
}


void corr(const vx_image a, const vx_image b, vx_image result) {
    const uint32_t width = a->width;
    const uint32_t height = a->height;
    const uint8_t* a_data = a->data;
    const uint8_t* b_data = b->data;
}


void norm(const vx_image src_image, const vx_image tmpl_image, vx_image dst_image) {
    const uint32_t src_width = src_image->width;
    const uint32_t src_height = src_image->height;
    const uint32_t tmpl_width = tmpl_image->width;
    const uint32_t tmpl_height = tmpl_image->height;
    const uint8_t* src_data = src_image->data;
    const uint8_t* tmpl_data = tmpl_image->data;
    uint8_t* dst_data = dst_image->data;
    
    for (uint32_t start_y = 0; start_y < src_height - tmpl_height; ++start_y) {
        for (uint32_t start_x = 0; start_x < src_width - tmpl_width; ++start_x) {
            uint64_t src_sum = 0;
            uint64_t tmpl_sum = 0;
        
            for (uint32_t y = 0; y < tmpl_height; ++y) {
                for (uint32_t x = 0; x < tmpl_width; ++x) {
                    src_sum += pow(dst_data[start_y * src_width + start_x  + src_width * y + x], 2);
                    tmpl_sum += pow(tmpl_data[y * tmpl_width + x], 2);
                }
            }

            double old_value = (double)dst_data[start_y * src_width + start_x];
            double norm_coef = sqrt(src_sum * tmpl_sum);
            dst_data[start_y * src_width + start_x] = (uint32_t)(old_value * pow(10, 6) / norm_coef);            
        }
    }
}