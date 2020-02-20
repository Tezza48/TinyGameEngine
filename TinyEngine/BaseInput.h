#pragma once

#include "Subject.h"
#include "Key.h"

namespace TinyEngine
{
	class BaseInput :
		public IObserver
	{
	public:
		// Inherited via Observer
		virtual void OnNotify(const Event& event) override;

	private:
		virtual void OnKeyDown(Key key) = 0;
		virtual void OnKeyUp(Key key) = 0;
		virtual void OnMouseMove(float x, float y) = 0;
	};
}

