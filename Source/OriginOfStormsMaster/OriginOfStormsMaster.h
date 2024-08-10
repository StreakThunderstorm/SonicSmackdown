// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

// Generally needs to be used as a pointer
#define GETENUMSTRING(etypename, evalue) ( (FindObject<UEnum>(ANY_PACKAGE, TEXT(etypename), true) != nullptr) ? FindObject<UEnum>(ANY_PACKAGE, TEXT(etypename), true)->GetEnumName((int32)evalue) : FString("Invalid - are you sure enum uses UENUM() macro?") )