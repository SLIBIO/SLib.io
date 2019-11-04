/*
 *   Copyright (c) 2008-2018 SLIBIO <https://github.com/SLIBIO>
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a copy
 *   of this software and associated documentation files (the "Software"), to deal
 *   in the Software without restriction, including without limitation the rights
 *   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *   copies of the Software, and to permit persons to whom the Software is
 *   furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *   THE SOFTWARE.
 */

#ifndef CHECKHEADER_SLIB_DEVICE_DEVICE
#define CHECKHEADER_SLIB_DEVICE_DEVICE

#include "definition.h"

#include "constants.h"

#include "../core/string.h"
#include "../core/list.h"
#include "../core/function.h"
#include "../math/size.h"

namespace slib
{

	typedef Function<void(const String& callId, const String& phoneNumber)> PhoneCallCallback;

	class SLIB_EXPORT Device
	{
	public:
		// Works on iOS
		static void setAudioCategory(const DeviceAudioCategory& category);
		
		
		// Works on Android
		static String getIMEI();
		
		// Works on Android
		static List<String> getIMEIs();

		// Works on Android
		static String getPhoneNumber();

		// Works on Android
		static List<String> getPhoneNumbers();
		
		static String getDeviceId();
		
		static String getDeviceName();
		
		static String getSystemVersion();
		
		static String getSystemName();
		
		static double getScreenPPI();
		
		static Sizei getScreenSize();
		
		static sl_uint32 getScreenWidth();
		
		static sl_uint32 getScreenHeight();
		
		
		static void openUrl(const String& url);


		static void openDial(const String& phoneNumber);

		static void callPhone(const String& phoneNumber);
		
		
		static void answerCall(const String& callId);

		static void endCall(const String& callId);

		
		static void addOnIncomingCall(const PhoneCallCallback& callback);
		
		static void removeOnIncomingCall(const PhoneCallCallback& callback);

		static void addOnOutgoingCall(const PhoneCallCallback& callback);
		
		static void removeOnOutgoingCall(const PhoneCallCallback& callback);
		
		static void addOnEndCall(const PhoneCallCallback& callback);
		
		static void removeOnEndCall(const PhoneCallCallback& callback);

	};

}

#endif
