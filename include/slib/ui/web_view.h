/*
 *  Copyright (c) 2008-2017 SLIBIO. All Rights Reserved.
 *
 *  This file is part of the SLib.io project.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CHECKHEADER_SLIB_UI_WEB_VIEW
#define CHECKHEADER_SLIB_UI_WEB_VIEW

#include "definition.h"

#ifndef SLIB_UI_IS_GTK

#include "view.h"

namespace slib
{
	
	class SLIB_EXPORT WebView : public View
	{
		SLIB_DECLARE_OBJECT
		
	public:
		WebView();
		
		~WebView();

	public:
		virtual void loadURL(const String& url);
		
		virtual void loadHTML(const String& html, const String& baseURL);
		
		sl_bool isOfflineContent();
		
		String getOriginURL();
		
		String getURL();
		
		String getPageTitle();
		
		virtual void goBack();
		
		virtual void goForward();
		
		virtual void reload();
		
		virtual void runJavaScript(const String& script);
		
		String getErrorMessage();
		
	public:
		SLIB_PROPERTY(AtomicFunction<void(WebView*, String)>, OnStartLoad)
		
		SLIB_PROPERTY(AtomicFunction<void(WebView*, String, sl_bool)>, OnFinishLoad)
		
		SLIB_PROPERTY(AtomicFunction<void(WebView*, String, String)>, OnMessageFromJavaScript)
		
	protected:
		virtual void onStartLoad(const String& url);
		
		virtual void onFinishLoad(const String& url, sl_bool flagFailed);
		
		virtual void onMessageFromJavaScript(const String& msg, const String& param);
		
		void onResize(sl_ui_len width, sl_ui_len height) override;
		
	public:
		Ref<ViewInstance> createNativeWidget(ViewInstance* parent) override;
		
		virtual void dispatchStartLoad(const String& url);
		
		virtual void dispatchFinishLoad(const String& url, sl_bool flagFailed);
		
		virtual void dispatchMessageFromJavaScript(const String& msg, const String& param);
		
	private:
		void _refreshSize_NW();
		
		void _load_NW();
		
		String _getURL_NW();
		
		String _getPageTitle_NW();
		
		void _goBack_NW();
		
		void _goForward_NW();
		
		void _reload_NW();
		
		void _runJavaScript_NW(const String& script);
		
	protected:
		AtomicString m_urlOrigin;
		AtomicString m_offlineContentHTML;
		sl_bool m_flagOfflineContent;
		AtomicString m_lastErrorMessage;
		
	};

}

#endif

#endif
