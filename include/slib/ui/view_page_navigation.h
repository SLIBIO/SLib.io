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

#ifndef CHECKHEADER_SLIB_VIEW_PAGE_NAVIGATION
#define CHECKHEADER_SLIB_VIEW_PAGE_NAVIGATION

#include "definition.h"

#include "view.h"
#include "transition.h"

namespace slib
{

	class ViewPage;

	class SLIB_EXPORT ViewPageNavigationController : public ViewGroup
	{
		SLIB_DECLARE_OBJECT
		
	public:
		ViewPageNavigationController();
		
		~ViewPageNavigationController();

	public:
		sl_size getPagesCount();
		
		Ref<View> getCurrentPage();
		
		void push(const Ref<View>& page, const Transition& transition, sl_bool flagRemoveAllBackPages = sl_false);

		void push(const Ref<View>& page, sl_bool flagRemoveAllBackPages = sl_false);
		
		void pop(const Ref<View>& page, const Transition& transition);
		
		void pop(const Ref<View>& page);
		
		void pop(const Transition& transition);
		
		void pop();
		
		void setSwipeNavigation(sl_bool flag);
		
		TransitionType getPushTransitionType();
		
		void setPushTransitionType(TransitionType type);
		
		TransitionType getPopTransitionType();
		
		void setPopTransitionType(TransitionType type);
		
		void setTransitionType(TransitionType type);
		
		TransitionDirection getPushTransitionDirection();
		
		void setPushTransitionDirection(TransitionDirection direction);
		
		TransitionDirection getPopTransitionDirection();
		
		void setPopTransitionDirection(TransitionDirection direction);
		
		void setTransitionDirection(TransitionDirection direction);
		
		float getPushTransitionDuration();
		
		void setPushTransitionDuration(float duration);

		float getPopTransitionDuration();
		
		void setPopTransitionDuration(float duration);
		
		void setTransitionDuration(float duration);
		
		AnimationCurve getPushTransitionCurve();
		
		void setPushTransitionCurve(AnimationCurve curve);
		
		AnimationCurve getPopTransitionCurve();
		
		void setPopTransitionCurve(AnimationCurve curve);
		
		void setTransitionCurve(AnimationCurve curve);
		
	protected:
		void _onFinishAnimation(const Ref<View>& page, UIPageAction action);
		
		void _resetAnimationStatus(const Ref<View>& page);
		
		void _push(const Ref<View>& page, const Transition& transition, sl_bool flagRemoveAllBackPages);
		
		void _pop(const Ref<View>& page, const Transition& transition);
		
		void _applyDefaultPushTransition(Transition& transition);
		
		void _applyDefaultPopTransition(Transition& transition);
		
		static void _runAnimationProc(const Ref<View>& page, const Function<void()>& callback);
		
	public:
		SLIB_DECLARE_EVENT_HANDLER(ViewPageNavigationController, PageAction, View* page, UIPageAction action)
		SLIB_DECLARE_EVENT_HANDLER(ViewPageNavigationController, EndPageAnimation, View* page, UIPageAction action)

	protected:
		void onResize(sl_ui_len width, sl_ui_len height) override;
		
		void onChangePadding() override;
		
		void onSwipe(GestureEvent* ev) override;
		
	protected:
		CList< Ref<View> > m_pages;

		sl_bool m_flagSwipeNavigation;

		TransitionType m_pushTransitionType;
		TransitionType m_popTransitionType;
		TransitionDirection m_pushTransitionDirection;
		TransitionDirection m_popTransitionDirection;
		float m_pushTransitionDuration; // seconds
		float m_popTransitionDuration; // seconds
		AnimationCurve m_pushTransitionCurve;
		AnimationCurve m_popTransitionCurve;
		
		sl_reg m_countActiveTransitionAnimations;
		
		friend class ViewPage;
		
	};

}

#endif