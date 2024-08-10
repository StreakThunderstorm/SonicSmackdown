#include "MNEditorSettings.h"

UMNEditorSettings::UMNEditorSettings()
{
	AutoLayoutStrategy = EMNAutoLayout::Tree;

	bFirstPassOnly = false;

	bRandomInit = false;

	OptimalDistance = 100.f;

	MaxIteration = 50;

	InitTemperature = 10.f;

	CoolDownRate = 10.f;
}

UMNEditorSettings::~UMNEditorSettings()
{

}

