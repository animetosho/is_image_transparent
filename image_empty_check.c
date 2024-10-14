/* compile using: gcc -O2 -lwebp -lpng image_empty_check.c */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h> // Include string.h for memcmp
#ifndef NO_WEBP
#include <webp/decode.h>
#endif
#ifndef NO_PNG
#include <png.h>
#endif

#define ERROR(...) do { fprintf(stderr, __VA_ARGS__); return 2; } while(0)
/* disallow loading images >32MB in size */
#ifndef MAX_SIZE
#define MAX_SIZE 33554432
#endif
#ifndef MAX_IMAGE_SIZE
#define MAX_IMAGE_SIZE 134217728
#endif

#define SIG_PNG "\x89PNG\r\n\x1A\n"  // Full PNG signature (8 bytes)
#define SIG_WEBP "RIFF"

int main(int argc, char** argv) {
	/* read input */
	if(argc != 2)
		ERROR("Usage: %s [image_file]\n", argv[0]);
	
	FILE* fp = fopen(argv[1], "rb");
	if(!fp)
		ERROR("Failed to open input file\n");
	
	fseek(fp, 0, SEEK_END);
	size_t size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	
	if(size > MAX_SIZE) {
		fclose(fp);
		ERROR("File size limited exceeded\n");
	}
	
	uint8_t* buf = malloc(size);
	size_t read = fread(buf, size, 1, fp);
	fclose(fp);
	
	if(!read) {
		free(buf);
		ERROR("Failed to read file\n");
	}
	
	
	/* Render */
	int width, height;
	uint8_t* image = NULL;
#ifndef NO_PNG
	if(memcmp(buf, SIG_PNG, 8) == 0) {
		png_image png;
		png.version = PNG_IMAGE_VERSION;
		png.opaque = NULL;
		if(!png_image_begin_read_from_memory(&png, buf, size)) {
			free(buf);
			ERROR("%s\n", png.message);
		}
		
		if(!(png.format & PNG_FORMAT_FLAG_ALPHA)) {
			free(buf);
			png_image_free(&png);
			return 1;
		}
		
		png.format = PNG_FORMAT_ARGB;
		if(PNG_IMAGE_SIZE(png) > MAX_IMAGE_SIZE) {
			free(buf);
			ERROR("Image too large\n");
		}
		image = malloc(PNG_IMAGE_SIZE(png));
		
		int success = png_image_finish_read(&png, NULL, image, 0, NULL);
		png_image_free(&png);
		if(!success) {
			free(image);
			free(buf);
			ERROR("%s\n", png.message);
		}
		
		width = png.width;
		height = png.height;
	} else
#endif
#ifndef NO_WEBP
	if(memcmp(buf, SIG_WEBP, 4) == 0) {
		WebPBitstreamFeatures webp;
		if(WebPGetFeatures(buf, size, &webp) != VP8_STATUS_OK) {
			free(buf);
			ERROR("Invalid WEBP file\n");
		}
		if(!webp.has_alpha) {
			free(buf);
			return 1;
		}
		if(webp.width * webp.height * 4 > MAX_IMAGE_SIZE) {
			free(buf);
			ERROR("Image too large\n");
		}
		image = WebPDecodeARGB(buf, size, &width, &height);
	} else
#endif
	{
		free(buf);
		ERROR("Invalid PNG/WEBP file\n");
	}
	free(buf);
	if(!image)
		ERROR("Failed to render supplied file\n");
	
	/* Transparency check */
	int result = 0;
	long i = width * height * 4;
	while(i) {
		i -= 4;
		if(image[i] != 0) {
			result = 1;
			break;
		}
	}
	
	free(image);
	
	return result;
}
