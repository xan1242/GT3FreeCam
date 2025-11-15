#pragma once
#include "stdafx.h"
#include "Matrix4.hpp"

#ifndef FREECAM_HPP
#define FREECAM_HPP

namespace FreeCam
{
	namespace Controls
	{
		struct ControlPacket
		{
			float movementValX = 0.0f;
			float movementValY = 0.0f;
			float movementValZ = 0.0f;
			float pitchVal = 0.0f;
			float yawVal = 0.0f;
			float rollVal = 0.0f;
		};

		bool GetToggleKeyState();
		ControlPacket GetKeyboardValues();
		ControlPacket GetControllerValues();
		bool GetControllerPresence();
		void UpdateState();
	}

	namespace Patches
	{
		bool CheckNeedsInstalling();
		bool Install();
	}

	void SetCameraUpdateState(bool state);
	bool GetCameraUpdateState();
	void SetState(bool state);
	bool GetState();
	uint32_t GetMatrixAddr();
	bool GetCameraMatrix(Matrix4& out, uint32_t inPS2Addr = 0);
	void SetCameraMatrix(const Matrix4& in, uint32_t inPS2Addr = 0);
	void ProcessControlPacket(const Controls::ControlPacket& pkt, Matrix4Ex& mat);
}

#endif