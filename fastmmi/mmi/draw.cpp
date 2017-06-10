/*
 * Copyright (c) 2013-2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include "layout.h"
#include "mmi.h"
#include "lang.h"
#include "graphics.h"
#include "utils.h"

static sem_t g_sem_draw;
static gr_text_func g_text;
static bool is_multi_enable = false;

void clear_screen(void) {
    gr_color(0, 0, 0, 255);
    gr_fill(0, 0, gr_fb_width(), gr_fb_height());
}

/*
 * To refresh screen
 */
void invalidate() {
    sem_post(&g_sem_draw);
}

void init_draw() {
    sem_init(&g_sem_draw, 0, 0);
    gr_init();

    is_multi_enable = !gr_init_multi();
    if(is_multi_enable)
        g_text = gr_text_multi;
    else
        g_text = gr_text;
}

static void align_center(rect_t * rect, int content_w, int content_h, point_t * point) {
    if(rect == NULL || point == NULL)
        return;

    if(content_h > rect->h && content_w > rect->w) {
        point->x = rect->x;
        point->y = rect->y;
    } else if(content_h > rect->h && content_w < rect->w) {
        point->x = rect->x + rect->w / 2 - content_w / 2;
        point->y = rect->y;
    } else if(content_h < rect->h && content_w > rect->w) {
        point->x = rect->x;
        point->y = rect->y + rect->h / 2 - content_h / 2;
    } else {
        point->x = rect->x + rect->w / 2 - content_w / 2;
        point->y = rect->y + rect->h / 2 - content_h / 2;
    }
}

/**Draw point*/
void draw_points(layout * lay) {

    if(lay == NULL)
        return;

    list < point_t >::iterator iter;
    for(iter = lay->trace.begin(); iter != lay->trace.end(); iter++) {
        point_t point = (point_t) (*iter);

        gr_color(255, 0, 0, 255);
        gr_fill(point.x, point.y, point.x + 1, point.y + 1);
    }
}

/**draw button*/
void draw_buttons(layout * lay) {
    if(lay == NULL)
        return;

    int font_x, font_y;

    if(is_multi_enable)
        gr_font_size_multi(&font_x, &font_y);
    else
        gr_font_size(&font_x, &font_y);

    list < button * >::iterator iter;
    for(iter = lay->button_list.begin(); iter != lay->button_list.end(); iter++) {
        button *btn = (button *) (*iter);

        if(btn->get_visibility()) {
            rect_t *rect = btn->get_rect();
            point_t point;
            color_t *btn_color = btn->get_color();

            gr_color(btn_color->r, btn_color->g, btn_color->b, btn_color->a);
            gr_fill(rect->x, rect->y, rect->x + rect->w, rect->y + rect->h);

            if(strlen(btn->get_image()) != 0) {
                /*load image need to attention, some time cause problem */
                //   image_t *img = load_image(btn->get_image());
                //    align_center(rect,img->img_w,img->img_h,&point);
                //     gr_blit(img->surface, 0, 0, img->img_w,img->img_h,point.x,point.y);
            } else {
                gr_color(255, 255, 255, 255);
                align_center(rect, font_x * strlen(btn->get_text()), font_y, &point);
                g_text(point.x, point.y, btn->get_text(), 1);
            }
        }
    }
}

void draw_textviews(layout * lay) {
    char tmp[1024] = { 0 };
    int32_t last_text_x = 0;
    int32_t last_text_y = 0;
    int32_t font_x, font_y;

    if(lay == NULL)
        return;

    if(is_multi_enable)
        gr_font_size_multi(&font_x, &font_y);
    else
        gr_font_size(&font_x, &font_y);

    list < textview * >::iterator iter;
    for(iter = lay->textview_list.begin(); iter != lay->textview_list.end(); iter++) {
        textview *tv = (textview *) (*iter);
        rect_t *rect = tv->get_rect();

        last_text_x = rect->x;
        last_text_y = rect->y;

        char *p = tv->get_text();
        int line_num = count_char(p, '\n');
        int max_line = rect->h / (2 * font_y);
        int pos = 0;

        /**set point to the start line*/
        if(line_num > max_line) {
            pos = get_pos(p, line_num - max_line);
            p += pos;
        }

        char *ptr;

        while(*p != '\0') {
            ptr = tmp;

            while(*p != '\n' && *p != '\0') {
                *ptr++ = *p++;
            }

            if(*p == '\n')
                p++;

            *ptr = '\0';
             /**Only check Y not outof rect, X will cut off String*/
            if((last_text_y + font_y) < (rect->y + rect->h)) {
                g_text(last_text_x, last_text_y, tmp, 1);
            }

            /*move to next line */
            last_text_y += 2 * font_y;
        }
    }
}

void draw_listviews(layout * lay) {

    int ret = -1;

    if(lay == NULL || lay->m_listview == NULL)
        return;

    listview *container = lay->m_listview;
    rect_t *rect = container->get_rect();
    int h = rect->h / container->get_item_num();
    int image_width = 50;

    if(container->get_items() == NULL)
        return;

    list < item_t * >::iterator iter;
    for(iter = container->get_items()->begin(); iter != container->get_items()->end(); iter++) {
        image_t *image = NULL;
        item_t *obj = (item_t *) (*iter);

        if(obj->hidden == 1 || !is_point_in_rect(obj->rect.x, obj->rect.y, rect)
           || !is_point_in_rect(obj->rect.x, obj->rect.y + obj->rect.h, rect)) {

            continue;
        }

        if(obj->mod->result == FAILED) {
            image = load_image((string) "fail.png");
        } else if(obj->mod->result == SUCCESS) {
            image = load_image((string) "pass.png");
        }

        if(image != NULL) {
            image_width = image->img_w;
            gr_blit(image->surface, 0, 0, image->img_w, image->img_h, obj->rect.x,
                    obj->rect.y + h / 2 - image->img_h / 2);
        }

        gr_color(255, 255, 255, 255);   // text
        char text[128];

        snprintf(text, sizeof(text), "%d %s", obj->index, get_string(obj->mod->config_list[KEY_DISPALY_NAME]));
        g_text(obj->rect.x + image_width + gr_fb_width() / 60, obj->rect.y + h / 2, text, 1);

        gr_color(255, 255, 255, 100);   // delimiter
        gr_fill(obj->rect.x, obj->rect.y + h, obj->rect.x + rect->w, obj->rect.y + h + 1);
    }
}

void *draw_thread(void *) {
    signal(SIGUSR1, signal_handler);
    while(1) {
        sem_wait(&g_sem_draw);
        layout *lay = acquire_cur_layout();

        clear_screen();
        draw_buttons(lay);
        draw_textviews(lay);
        draw_listviews(lay);
        draw_points(lay);
        release_cur_layout();
        gr_flip();
    }
    return NULL;
}
