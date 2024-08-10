// Fill out your copyright notice in the Description page of Project Settings.

#include "MNNode_Move.h"

EOOSInputDir UMNNode_Move::GetInputDir() const
{
	EOOSInputDir Result = EOOSInputDir::OOSID_None;
	switch (Move.Direction)
	{
	case EOOSMoveDir::OOSMD_Forward:
		Result = EOOSInputDir::OOSID_Right;
		break;
	case EOOSMoveDir::OOSMD_UpForward:
		Result = EOOSInputDir::OOSID_UpRight;
		break;
	case EOOSMoveDir::OOSMD_Up:
		Result = EOOSInputDir::OOSID_Up;
		break;
	case EOOSMoveDir::OOSMD_UpBack:
		Result = EOOSInputDir::OOSID_UpLeft;
		break;
	case EOOSMoveDir::OOSMD_Back:
		Result = EOOSInputDir::OOSID_Left;
		break;
	case EOOSMoveDir::OOSMD_DownBack:
		Result = EOOSInputDir::OOSID_DownLeft;
		break;
	case EOOSMoveDir::OOSMD_Down:
		Result = EOOSInputDir::OOSID_Down;
		break;
	case EOOSMoveDir::OOSMD_DownForward:
		Result = EOOSInputDir::OOSID_DownRight;
		break;
	case EOOSMoveDir::OOSMD_None:
	default:
		Result = EOOSInputDir::OOSID_None;
		break;
	}

	return Result;
}
