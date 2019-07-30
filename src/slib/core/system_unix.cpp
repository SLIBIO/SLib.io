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

#include "slib/core/definition.h"

#if defined(SLIB_PLATFORM_IS_UNIX)

#include "slib/core/system.h"
#include "slib/core/file.h"
#include "slib/core/console.h"
#include "slib/core/log.h"
#include "slib/core/list.h"
#include "slib/core/safe_static.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <errno.h>

#include <unistd.h>
#include <sched.h>
#include <signal.h>
#include <fcntl.h>
#include <termios.h>
#include <pwd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/syscall.h>

#define PRIV_PATH_MAX 1024

#if defined(SLIB_PLATFORM_IS_ANDROID)

#include "slib/core/platform_android.h"

namespace slib
{
	namespace priv
	{
		namespace system
		{
			SLIB_JNI_BEGIN_CLASS(JAndroid, "slib/platform/android/Android")
				SLIB_JNI_STATIC_METHOD(getDeviceNameOnSettings, "getDeviceNameOnSettings", "(Landroid/app/Activity;)Ljava/lang/String;")
			SLIB_JNI_END_CLASS
		}
	}
}

#endif

namespace slib
{

#if !defined(SLIB_PLATFORM_IS_APPLE)
	String System::getApplicationPath()
	{
		char path[PRIV_PATH_MAX] = {0};
#	if defined(SLIB_PLATFORM_IS_ANDROID)
		char a[50];
		::sprintf(a, "/proc/%d/cmdline", getpid());
		FILE* fp = fopen(a, "rb");
		int n = fread(path, 1, PRIV_PATH_MAX - 1, fp);
		fclose(fp);

		String ret;
		if (n > 0) {
			ret = String::fromUtf8(path);
		}
		return "/data/data/" + ret;
#	else
		int n = readlink("/proc/self/exe", path, PRIV_PATH_MAX-1);
		/*
		-- another solution --

			char a[50];
			sprintf(a, "/proc/%d/exe", getpid());
			int n = readlink(a, path, size);
		*/
		String ret;
		if (n > 0) {
			ret = String::fromUtf8(path, n);
		}
		return ret;
#	endif
	}

	String System::getHomeDirectory()
	{
#	if defined(SLIB_PLATFORM_IS_MOBILE)
		return getApplicationDirectory();
#	else
		passwd* pwd = getpwuid(getuid());
		return pwd->pw_dir;
#	endif
	}

	String System::getTempDirectory()
	{
#	if defined(SLIB_PLATFORM_IS_MOBILE)
		String dir = System::getApplicationDirectory() + "/temp";
		File::createDirectory(dir);
		return dir;
#	else
		return "/tmp";
#	endif
	}
#endif

	String System::getCurrentDirectory()
	{
		char path[PRIV_PATH_MAX] = {0};
		char* r = getcwd(path, PRIV_PATH_MAX-1);
		if (r) {
			return path;
		}
		return sl_null;
	}

	sl_bool System::setCurrentDirectory(const String& dir)
	{
		int iRet = chdir(dir.getData());
		if (iRet == 0) {
			return sl_true;
		}
		return sl_false;
	}
	
#if !defined(SLIB_PLATFORM_IS_APPLE)
	String System::getComputerName()
	{
#	if defined(SLIB_PLATFORM_IS_ANDROID)
		jobject jactivity = Android::getCurrentActivity();
		if (jactivity) {
			return priv::system::JAndroid::getDeviceNameOnSettings.callString(sl_null, jactivity);
		}
		return sl_null;
#	elif defined(SLIB_PLATFORM_IS_MOBILE)
		return "Mobile Device";
#	else
		char buf[512] = {0};
		gethostname(buf, 500);
		return buf;
#	endif
	}
	
	String System::getUserName()
	{
#	if defined(SLIB_PLATFORM_IS_MOBILE)
		return "mobile";
#	else
		return getlogin();
#	endif
	}
	
	String System::getFullUserName()
	{
#	if defined(SLIB_PLATFORM_IS_MOBILE)
		return "Mobile User";
#	else
		return getUserName();
#	endif
	}
#endif

	sl_uint32 System::getTickCount()
	{
		return (sl_uint32)(getTickCount64());
	}
	
#if !defined(SLIB_PLATFORM_IS_APPLE)
	sl_uint64 System::getTickCount64()
	{
#if defined(_POSIX_TIMERS) && _POSIX_TIMERS > 0 && defined(CLOCK_MONOTONIC)
		{
			static sl_bool flagCheck = sl_true;
			static sl_bool flagEnabled = sl_false;
			struct timespec ts;
			if (flagEnabled) {
				clock_gettime(CLOCK_MONOTONIC, &ts);
				return (sl_uint64)(ts.tv_sec) * 1000 + (sl_uint64)(ts.tv_nsec) / 1000000;
			} else {
				if (flagCheck) {
					int iRet = clock_gettime(CLOCK_MONOTONIC, &ts);
					if (iRet < 0) {
						flagCheck = sl_false;
					} else {
						flagEnabled = sl_true;
						flagCheck = sl_false;
						return (sl_uint64)(ts.tv_sec) * 1000 + (sl_uint64)(ts.tv_nsec) / 1000000;
					}
				}
			}
		}
#endif
		struct timeval tv;
		if (gettimeofday(&tv, 0) == 0) {
			return (sl_uint64)(tv.tv_sec) * 1000 + tv.tv_usec / 1000;
		} else {
			return 0;
		}
	}
#endif

	sl_uint32 System::getProcessId()
	{
		return getpid();
	}

	sl_uint32 System::getThreadId()
	{
#if defined(SLIB_PLATFORM_IS_APPLE)
		int ret = syscall(SYS_thread_selfid);
		return ret;
#elif defined(SLIB_PLATFORM_IS_ANDROID)
		pid_t tid = gettid();
		return tid;
#else
		pid_t tid = syscall(SYS_gettid);
		return tid;
#endif
	}

#if !defined(SLIB_PLATFORM_IS_MOBILE)
	sl_bool System::createProcess(const String& pathExecutable, const String* cmds, sl_uint32 nCmds)
	{
		pid_t pid = fork();
		if (pid == -1) {
			LogError("System::createProcess", "Cannot fork");
			return sl_false;
		} else {
			if (pid == 0) {
				// Child process

				// Daemonize
				setsid();
				close(0);
				close(1);
				close(2);
				open("/dev/null", O_RDWR);
				dup2(0, 1);
				dup2(0, 2);

				signal(SIGHUP, SIG_IGN);

				char* exe = pathExecutable.getData();
				char* args[1024];
				args[0] = exe;
				if (nCmds > 1020) {
					nCmds = 1020;
				}
				for (sl_size i = 0; i < nCmds; i++) {
					args[i+1] = cmds[i].getData();
				}
				args[nCmds+1] = 0;

				::execvp(exe, args);
				::exit(1);
				return sl_false;
			} else {
				// parent process
				return sl_true;
			}
		}
	}

	void System::exec(const String& pathExecutable, const String* cmds, sl_uint32 nCmds)
	{
		char* exe = pathExecutable.getData();
		char* args[1024];
		args[0] = exe;
		if (nCmds > 1020) {
			nCmds = 1020;
		}
		for (sl_size i = 0; i < nCmds; i++) {
			args[i+1] = cmds[i].getData();
		}
		args[nCmds+1] = 0;
		
		::execvp(exe, args);
		::exit(1);
	}

	void System::exit(int code)
	{
		::exit(code);
	}
#endif

	void System::sleep(sl_uint32 milliseconds)
	{
		struct timespec req;
		req.tv_sec = milliseconds / 1000;
		req.tv_nsec = (milliseconds % 1000) * 1000000;
		nanosleep(&req, sl_null);
	}

	void System::yield()
	{
		sched_yield();
	}
	
	sl_uint32 System::getLastError()
	{
		return errno;
	}
	
	String System::formatErrorCode(sl_uint32 errorCode)
	{
		String ret = ::strerror(errorCode);
		if (ret.isEmpty()) {
			return String::format("Unknown error: %d", errorCode);
		}
		return ret;
	}

	void System::abort(const String& msg, const String& file, sl_uint32 line)
	{
#if defined(SLIB_DEBUG)
#	if defined(SLIB_PLATFORM_IS_ANDROID)
		__assert(file.getData(), line, msg.getData());
#	else
		__assert(msg.getData(), file.getData(), line);
#	endif
#endif
	}

#if !defined(SLIB_PLATFORM_IS_MOBILE)
	namespace priv
	{
		namespace system
		{
			volatile double g_signal_fpe_dummy = 0.0f;
		}
	}

	void System::setCrashHandler(SIGNAL_HANDLER handler)
	{
		struct sigaction sa;
		sa.sa_flags = SA_NODEFER;
		sa.sa_handler = handler;
		sigemptyset(&(sa.sa_mask));
		sigaction(SIGFPE, &sa, NULL);
		sigaction(SIGSEGV, &sa, NULL);
		sigaction(SIGBUS, &sa, NULL);
		sigaction(SIGILL, &sa, NULL);
		sigaction(SIGABRT, &sa, NULL);
		sigaction(SIGIOT, &sa, NULL);
#	if defined(SLIB_PLATFORM_IS_MACOS)
		sigaction(SIGEMT, &sa, NULL);
#	endif
		sigaction(SIGSYS, &sa, NULL);
	}
#endif

#if !defined(SLIB_PLATFORM_IS_MOBILE)
	
	namespace priv
	{
		namespace system
		{
			
			typedef CList<String> GlobalUniqueInstanceList;
			
			SLIB_SAFE_STATIC_GETTER(GlobalUniqueInstanceList, getGlobalUniqueInstanceList)
			
			class GlobalUniqueInstanceImpl : public GlobalUniqueInstance
			{
			public:
				String m_name;
				Ref<File> m_file;
				String m_filePath;
				
			public:
				GlobalUniqueInstanceImpl()
				{
				}
				
				~GlobalUniqueInstanceImpl()
				{
					m_file->unlock();
					m_file->close();
					File::deleteFile(m_filePath);
					GlobalUniqueInstanceList* list = getGlobalUniqueInstanceList();
					if (list) {
						list->remove(m_name);
					}
				}
				
			};

		}
	}

	Ref<GlobalUniqueInstance> GlobalUniqueInstance::create(const String& _name)
	{
		if (_name.isEmpty()) {
			return sl_null;
		}
		
		String name = File::makeSafeFileName(_name);
		priv::system::GlobalUniqueInstanceList* list = priv::system::getGlobalUniqueInstanceList();
		if (!list) {
			return sl_null;
		}
		if (list->indexOf(name) >= 0) {
			return sl_null;
		}
		
		Ref<priv::system::GlobalUniqueInstanceImpl> instance = new priv::system::GlobalUniqueInstanceImpl;
		if (instance.isNotNull()) {
			String filePath = "/tmp/.slib_global_lock_" + name;
			Ref<File> file = File::openForWrite(filePath);
			if (file.isNotNull()) {
				if (file->lock()) {
					instance->m_name = name;
					instance->m_file = file;
					instance->m_filePath = filePath;
					list->add(name);
					return instance;
				}
				file->close();
				File::deleteFile(filePath);
			}
		}
		return sl_null;
	}

	sl_bool GlobalUniqueInstance::exists(const String& _name)
	{
		if (_name.isEmpty()) {
			return sl_false;
		}
		String name = File::makeSafeFileName(_name);
		priv::system::GlobalUniqueInstanceList* list = priv::system::getGlobalUniqueInstanceList();
		if (!list) {
			return sl_false;
		}
		if (list->indexOf(name) >= 0) {
			return sl_true;
		}
		String fileName = "/tmp/.slib_global_lock_" + name;
		if (File::exists(fileName)) {
			Ref<File> file = File::openForWrite(fileName);
			if (file.isNull()) {
				return sl_true;
			} else {
				if (file->lock()) {
					file->unlock();
				} else {
					return sl_true;
				}
				file->close();
				File::deleteFile(fileName);
			}
		}
		return sl_false;
	}

#endif

	namespace priv
	{
		void Abort(const char* msg, const char* file, sl_uint32 line) noexcept
		{
#if defined(SLIB_DEBUG)
#	if defined(SLIB_PLATFORM_IS_ANDROID)
			__assert(file, line, msg);
#	else
			__assert(msg, file, line);
#	endif
#endif
		}
	}

	void Console::print(const String& s)
	{
#if defined(SLIB_PLATFORM_IS_ANDROID) || defined(SLIB_PLATFORM_IS_TIZEN)
		SLIB_STATIC_STRING(c, "Console");
		Logger::getConsoleLogger()->log(c, s);
#else
		String _s = s;
		printf("%s", _s.getData());
#endif
	}

#if !defined(SLIB_PLATFORM_IS_MOBILE)
	String Console::readLine()
	{
		String ret;
		char* line = NULL;
		size_t len = 0;
		::getline(&line, &len, stdin);
		if (line) {
			ret = String(line, Base::getStringLength(line, (sl_int32)len));
			::free(line);
		}
		return ret;
	}
	
	sl_char16 Console::readChar(sl_bool flagPrintEcho)
	{
		termios tOld, tNew;
		::tcgetattr(0, &tOld);
		tNew = tOld;
		tNew.c_lflag &= ~ICANON;
		if (flagPrintEcho) {
			tNew.c_lflag |= ECHO;
		} else {
			tNew.c_lflag &= ~ECHO;
		}
		::tcsetattr(0, TCSANOW, &tNew);
		sl_char16 ch = (sl_char16)(::getchar());
		::tcsetattr(0, TCSANOW, &tOld);
		return ch;
	}
#endif

}

#endif
