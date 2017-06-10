#ifndef __GEMINI_APP_H__
#define __GEMINI_APP_H__

struct test_gemini_param {
	/* command line parameters */
	int yuv_w;
	int yuv_h;
	char *input_yuv_filename;
	char *output_bs_filename;
	char *output_jpeg_filename;

	int input_byte_ordering;
	int output_byte_ordering;
	int cbcr_ordering;
	int zero_y;
	int zero_cbcr;
	int output_quality;

	/* parameters passed back from gemini app */
	gmn_obj_t gmn_obj;
	int gmnfd;
};

#endif /* __GEMINI_APP_H__ */
