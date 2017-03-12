/* compile using: gcc -O2 -lwebp -lpng image_empty_check.c */

#include <stdio.h>
#include <stdlib.h>
#include <webp/decode.h>
#include <png.h>

#define ERROR(...) do { fprintf(stderr, __VA_ARGS__); return 2; } while(0)
/* disallow loading images >32MB in size */
#define MAX_SIZE 33554432

#define SIG_PNG (*(uint32_t*)"\x89PNG")
#define SIG_WEBP (*(uint32_t*)"RIFF")

uint8_t* decode_png(const uint8_t* buf, size_t length, int* width, int* height) {
	png_image png;
	png.version = PNG_IMAGE_VERSION;
	png.opaque = NULL;
	if(!png_image_begin_read_from_memory(&png, buf, length)) {
		return NULL;
	}
	
	png.format = PNG_FORMAT_ARGB;
	uint8_t* image = malloc(PNG_IMAGE_SIZE(png));
	
	if(!png_image_finish_read(&png, NULL, image, 0, NULL)) {
		free(image);
		image = NULL;
	}
	png_image_free(&png);
	
	*width = png.width;
	*height = png.height;
	return image;
}

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
	/* TODO: consider checking image format to see if alpha is even possible, and bail early if it isn't */
	uint32_t sig = *(uint32_t*)buf;
	int width, height;
	uint8_t* image;
	if(sig == SIG_PNG) {
		image = decode_png(buf, size, &width, &height);
	} else if(sig == SIG_WEBP) {
		image = WebPDecodeARGB(buf, size, &width, &height);
	} else {
		free(buf);
		ERROR("Invalid PNG/WEBP file\n");
	}
	if(!image) {
		free(buf);
		ERROR("Failed to render supplied file\n");
	}
	
	/* transparency check */
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
	free(buf);
	
	return result;
}
