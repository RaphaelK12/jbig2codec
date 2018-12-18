// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/codec/ccodec_jbig2module.h"

#include <list>
#include <memory>

#include "core/fpdfapi/parser/cpdf_stream_acc.h"
#include "core/fxcodec/JBig2_DocumentContext.h"
#include "core/fxcodec/jbig2/JBig2_Context.h"
#include "core/fxcodec/jbig2/JBig2_Image.h"
#include "third_party/base/ptr_util.h"

JBig2_DocumentContext::JBig2_DocumentContext() {}

JBig2_DocumentContext::~JBig2_DocumentContext() {}

JBig2_DocumentContext *GetJBig2DocumentContext(
        std::unique_ptr<JBig2_DocumentContext> *pContextHolder)
{
	if (!pContextHolder->get()) {
		*pContextHolder = pdfium::MakeUnique<JBig2_DocumentContext>();
	}
	return pContextHolder->get();
}

CCodec_Jbig2Context::CCodec_Jbig2Context()
	: m_width(0),
	  m_height(0),
	  m_dest_buf(0),
	  m_dest_pitch(0) {}

CCodec_Jbig2Context::~CCodec_Jbig2Context() {}

CCodec_Jbig2Module::~CCodec_Jbig2Module() {}

FXCODEC_STATUS CCodec_Jbig2Module::StartDecode(
        CCodec_Jbig2Context *pJbig2Context,
        std::unique_ptr<JBig2_DocumentContext> *pContextHolder,
        uint8_t *src_buf, uint32_t src_len,
        uint32_t width, uint32_t height,
        uint8_t *dest_buf, uint32_t dest_pitch, uint32_t len,
        int32_t decode_type)
{
	if (!pJbig2Context) {
		return FXCODEC_STATUS_ERR_PARAMS;
	}

	JBig2_DocumentContext *doc_ctx =
	        GetJBig2DocumentContext(pContextHolder);
	pJbig2Context->m_width = width;
	pJbig2Context->m_height = height;
	pJbig2Context->m_dest_buf = dest_buf;
	pJbig2Context->m_dest_pitch = dest_pitch;
	memset(dest_buf, 0, height * dest_pitch);

	pJbig2Context->m_pContext = pdfium::MakeUnique<CJBig2_Context>(
	                                    src_buf, src_len, 75, doc_ctx->GetSymbolDictCache());
	pJbig2Context->m_pContext->m_PauseStep = decode_type;

	bool succeeded = pJbig2Context->m_pContext->GetFirstPage(
	                         dest_buf, width, height, dest_pitch, nullptr);
	return Decode(pJbig2Context, succeeded);
}

FXCODEC_STATUS CCodec_Jbig2Module::ContinueDecode(
        CCodec_Jbig2Context *pJbig2Context,
        PauseIndicatorIface *pPause)
{
	bool succeeded = pJbig2Context->m_pContext->Continue(pPause);
	return Decode(pJbig2Context, succeeded);
}

FXCODEC_STATUS CCodec_Jbig2Module::Decode(CCodec_Jbig2Context *pJbig2Context,
                bool decode_success)
{
	FXCODEC_STATUS status = pJbig2Context->m_pContext->GetProcessingStatus();
	if (status != FXCODEC_STATUS_DECODE_FINISH) {
		return status;
	}

	pJbig2Context->m_pContext.reset();
	if (!decode_success) {
		return FXCODEC_STATUS_ERROR;
	}
	return FXCODEC_STATUS_DECODE_FINISH;
}
