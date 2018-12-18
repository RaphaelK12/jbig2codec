/*
 * Copyright (c) 2018, Avia Systems Limited.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Avia Systems Limited or its parts including
 *       Scan2CAD nor the names of its contributors may be used to endorse or
 *       promote products derived from this software without specific prior
 *       written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * This is a JBIG2 codec compiled as a standalone program. It can be used this
 * way:
 *
 *     $ jbig2codec <input_jbig2_file> <width> <height> <output_pbm_file>
 */
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>

#include "core/fxcodec/codec/ccodec_jbig2module.h"
#include "core/fxcodec/JBig2_DocumentContext.h"
#include "core/fxcodec/jbig2/JBig2_Context.h"
#include "core/fxcodec/jbig2/JBig2_Image.h"
#include "third_party/base/ptr_util.h"

#define IMG_MAXBUFSZ		(256*1024*1024)
#define PS_DECODE_SEQUENTIAL	2
#define PS_DECODE_RANDOM	3

static uint8_t src[IMG_MAXBUFSZ];
static uint8_t dest[IMG_MAXBUFSZ];

static CCodec_Jbig2Module jb2_module;
static CCodec_Jbig2Context jb2_ctx;
static std::unique_ptr<JBig2_DocumentContext> doc_ctx;

int main(int argc, char *argv[])
{
	FILE *jb2, *out;
	FXCODEC_STATUS ds;
	uint32_t width, height, pitch;
	uint64_t src_len;
	uint64_t out_len;
	int32_t decode_type;
	int rc;

	sscanf(argv[2], "%" SCNu32, &width);
	sscanf(argv[3], "%" SCNu32, &height);
	if ((width%8) != 0) {
		pitch = (width/8)+1;
	} else {
		pitch = width/8;
	}
	out_len = height*pitch;
	rc = 0;

	jb2 = fopen(argv[1], "rb");
	if (jb2 == NULL) {
		printf("Cannot open: %s\n", argv[1]);
	} else {
		rc = fread(src, sizeof src[0], IMG_MAXBUFSZ, jb2);
		if (rc == IMG_MAXBUFSZ) {
			printf("Read exactly %d bytes from: %s\n",
			       IMG_MAXBUFSZ, argv[1]);
			src_len = rc;
			rc = 0;
		} else if (feof(jb2) != 0) {
			printf("Read %d bytes from: %s, when EOF occured\n",
			       rc, argv[1]);
			src_len = rc;
			rc = 0;
		} else if (ferror(jb2) != 0) {
			printf("Read %d bytes from: %s, when error occured\n",
			       rc, argv[1]);
			rc = 77;
		} else {
			printf("Read %d bytes from: %s, when something else "
			       "happened\n", rc, argv[1]);
			rc = 78;
		}
	}
	if (jb2 != NULL) {
		fclose(jb2);
	}

	if (rc == 0) {
		/* Try to use modified JBIG2 codec from the Pdfium project */
		doc_ctx = pdfium::MakeUnique<JBig2_DocumentContext>();
		decode_type = ((src[8]&1) == 0) ? PS_DECODE_RANDOM
		              : PS_DECODE_SEQUENTIAL;

		/* NOTE: Header of the JBIG2 file is 13 bytes long. */
		ds = jb2_module.StartDecode(
		             &jb2_ctx, &doc_ctx, &src[13], src_len-13,
		             width, height, dest, pitch, IMG_MAXBUFSZ,
		             decode_type);
		while (ds != FXCODEC_STATUS_DECODE_FINISH) {
			if (ds < 0) {
				rc = 75;
				break;
			}
			ds = jb2_module.ContinueDecode(&jb2_ctx, nullptr);
		}

		/* Write PBM file with the decoded image */
		out = fopen(argv[4], "wb");
		if (out == NULL) {
			printf("Cannot open %s to write to\n", argv[4]);
			rc = 76;
		} else {
			/* Write a PBM (monochrome) image header */
			fprintf(out, "P4\n# 1-bit image extracted by "
			        "jbig2codec\n%" PRIu32 " %" PRIu32 "\n",
			        width, height);
			/* Write an image data */
			for (uint64_t i = 0; i < out_len; i++) {
				fwrite(&dest[i], sizeof dest[i], 1, out);
			}
		}
	}
	return rc;
}
