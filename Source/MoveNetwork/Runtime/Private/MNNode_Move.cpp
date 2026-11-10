// Fill out your copyright notice in the Description page of Project Settings.

#include "MNNode_Move.h"

ESSDInputDir UMNNode_Move::GetInputDir() const
{
	ESSDInputDir Result = ESSDInputDir::SSDID_None;
	switch (Move.Direction)
	{
	case ESSDMoveDir::SSDMD_Forward:
		Result = ESSDInputDir::SSDID_Right;
		break;
	case ESSDMoveDir::SSDMD_UpForward:
		Result = ESSDInputDir::SSDID_UpRight;
		break;
	case ESSDMoveDir::SSDMD_Up:
		Result = ESSDInputDir::SSDID_Up;
		break;
	case ESSDMoveDir::SSDMD_UpBack:
		Result = ESSDInputDir::SSDID_UpLeft;
		break;
	case ESSDMoveDir::SSDMD_Back:
		Result = ESSDInputDir::SSDID_Left;
		break;
	case ESSDMoveDir::SSDMD_DownBack:
		Result = ESSDInputDir::SSDID_DownLeft;
		break;
	case ESSDMoveDir::SSDMD_Down:
		Result = ESSDInputDir::SSDID_Down;
		break;
	case ESSDMoveDir::SSDMD_DownForward:
		Result = ESSDInputDir::SSDID_DownRight;
		break;
	case ESSDMoveDir::SSDMD_None:
	default:
		Result = ESSDInputDir::SSDID_None;
		break;
	}

	return Result;
}
