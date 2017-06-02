/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "WKNavigationActionRef.h"

#include "APINavigationAction.h"
#include "WKAPICast.h"

using namespace WebKit;

WKTypeID WKNavigationActionGetTypeID()
{
    return toAPI(API::NavigationAction::APIType);
}

WK_EXPORT WKFrameNavigationType WKNavigationActionGetNavigationType(WKNavigationActionRef navigationActionRef)
{
    return toAPI(toImpl(navigationActionRef)->navigationType());
}

WK_EXPORT WKEventModifiers WKNavigationActionGetEventModifiers(WKNavigationActionRef navigationActionRef)
{
    return toAPI(toImpl(navigationActionRef)->modifiers());

}
WK_EXPORT WKEventMouseButton WKNavigationActionGetEventMouseButton(WKNavigationActionRef navigationActionRef)
{
    return toAPI(toImpl(navigationActionRef)->mouseButton());
}

bool WKNavigationActionGetCanHandleRequest(WKNavigationActionRef navigationActionRef)
{
    return toImpl(navigationActionRef)->canHandleRequest();
}

bool WKNavigationActionGetShouldOpenExternalSchemes(WKNavigationActionRef navigationActionRef)
{
    return toImpl(navigationActionRef)->shouldOpenExternalSchemes();
}

bool WKNavigationActionGetShouldOpenAppLinks(WKNavigationActionRef navigationActionRef)
{
    return toImpl(navigationActionRef)->shouldOpenAppLinks();
}

bool WKNavigationActionGetShouldPerformDownload(WKNavigationActionRef navigationActionRef)
{
    return toImpl(navigationActionRef)->shouldPerformDownload();
}

bool WKNavigationActionGetIsProcessingUserGesture(WKNavigationActionRef navigationActionRef)
{
    return toImpl(navigationActionRef)->isProcessingUserGesture();
}
