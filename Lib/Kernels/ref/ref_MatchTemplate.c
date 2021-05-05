#include "../ref.h"

/*
    Function: Вычисляет корреляцию между двумя картинками
    Parameters:
        a - левый операнд
        b - правый операнд
        result - результат, массив, в которой будет записана корреляция между a и b
*/
void corr(const vx_image a, const vx_image b, vx_image result);

vx_status ref_MatchTemplate(const vx_image src_image, const vx_image tmplt_image, match_template_method_enum method, vx_image dst_image) {
    const uint32_t src_width = src_image->width;
    const uint32_t src_height = src_image->height;
    const uint32_t tmpl_width = tmplt_image->width;
    const uint32_t tmpl_height = tmplt_image->height;
    const uint32_t dst_width = dst_image->width;
    const uint32_t dst_height = dst_image->height;

    if (src_width <  tmpl_width || src_height < tmpl_height || src_width != dst_width || src_height != dst_height) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    const uint8_t* src_data = src_image->data;
    const uint8_t* tmpl_data = tmplt_image->data;
    uint8_t* dst_data = dst_image->data;

    if (method == CCORR || method == CCORR_NORMED) {
        corr(src_image, tmplt_image, dst_image);

        if (method == CCORR_NORMED) {

        }
    }

    if (method == SQDIFF || method == SQDIFF_NORMED) {
        if (method == SQDIFF_NORMED) {

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