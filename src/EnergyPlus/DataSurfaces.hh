// EnergyPlus, Copyright (c) 1996-2021, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Sustainable Energy, LLC, and other
// contributors. All rights reserved.
//
// NOTICE: This Software was developed under funding from the U.S. Department of Energy and the
// U.S. Government consequently retains certain rights. As such, the U.S. Government has been
// granted for itself and others acting on its behalf a paid-up, nonexclusive, irrevocable,
// worldwide license in the Software to reproduce, distribute copies to the public, prepare
// derivative works, and perform publicly and display publicly, and to permit others to do so.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted
// provided that the following conditions are met:
//
// (1) Redistributions of source code must retain the above copyright notice, this list of
//     conditions and the following disclaimer.
//
// (2) Redistributions in binary form must reproduce the above copyright notice, this list of
//     conditions and the following disclaimer in the documentation and/or other materials
//     provided with the distribution.
//
// (3) Neither the name of the University of California, Lawrence Berkeley National Laboratory,
//     the University of Illinois, U.S. Dept. of Energy nor the names of its contributors may be
//     used to endorse or promote products derived from this software without specific prior
//     written permission.
//
// (4) Use of EnergyPlus(TM) Name. If Licensee (i) distributes the software in stand-alone form
//     without changes from the version obtained under this License, or (ii) Licensee makes a
//     reference solely to the software portion of its product, Licensee must refer to the
//     software as "EnergyPlus version X" software, where "X" is the version number Licensee
//     obtained under this License and may not use a different name for the software. Except as
//     specifically required in this Section (4), Licensee shall not use in a company name, a
//     product name, in advertising, publicity, or other promotional activities any name, trade
//     name, trademark, logo, or other designation of "EnergyPlus", "E+", "e+" or confusingly
//     similar designation, without the U.S. Department of Energy's prior written consent.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#ifndef DataSurfaces_hh_INCLUDED
#define DataSurfaces_hh_INCLUDED

// C++ Headers
#include <cstddef>
#include <vector>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Array2D.hh>
#include <ObjexxFCL/Vector4.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataBSDFWindow.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataVectorTypes.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Shape.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace DataSurfaces {

    // Using/Aliasing
    using DataBSDFWindow::BSDFWindowDescript;
    using DataVectorTypes::Vector;

    // MODULE PARAMETER DEFINITIONS:
    constexpr int MaxSlatAngs(19);

    // Parameters to indicate surface shape for use with the Surface
    // derived type (see below):

    enum class SurfaceShape : int
    {
        None = 0,
        Triangle,
        Quadrilateral,
        Rectangle,
        RectangularDoorWindow,
        RectangularOverhang,
        RectangularLeftFin,
        RectangularRightFin,
        TriangularWindow,
        TriangularDoor,
        Polygonal
    };

    enum class SurfaceClass : int
    {
        INVALID = -1,
        None = 0,
        Wall,
        Floor,
        Roof,
        IntMass,
        Detached_B,
        Detached_F,
        Window,
        GlassDoor,
        Door,
        Shading,
        Overhang,
        Fin,
        TDD_Dome,
        TDD_Diffuser,
        Count // The counter representing the total number of surface class, always stays at the bottom
    };

    // Parameters to indicate exterior boundary conditions for use with
    // the Surface derived type (see below):
    // Note:  Positive values correspond to an interzone adjacent surface

    constexpr int ExternalEnvironment(0);
    constexpr int Ground(-1);
    constexpr int OtherSideCoefNoCalcExt(-2);
    constexpr int OtherSideCoefCalcExt(-3);
    constexpr int OtherSideCondModeledExt(-4);
    constexpr int GroundFCfactorMethod(-5);
    constexpr int KivaFoundation(-6);

    extern Array1D_string const cExtBoundCondition;

    // Parameters to indicate the first "corner" of a surface
    // Currently, these are used only during input of surfaces
    // They are here in order to facilitate later use in shading setup/calculations.
    constexpr int UpperLeftCorner(1);
    constexpr int LowerLeftCorner(2);
    constexpr int LowerRightCorner(3);
    constexpr int UpperRightCorner(4);

    // Parameters to indicate user specified convection coefficients (for surface)
    constexpr int ConvCoefValue(1);          // User specified "value" as the override type
    constexpr int ConvCoefSchedule(2);       // User specified "schedule" as the override type
    constexpr int ConvCoefUserCurve(3);      // User specified "UserCurve" as the override type
    constexpr int ConvCoefSpecifiedModel(4); // one of the direct named model equation keys

    // Parameters to indicate reference air temperatures for inside surface temperature calculations
    constexpr int ZoneMeanAirTemp(1);   // mean air temperature of the zone => MAT
    constexpr int AdjacentAirTemp(2);   // air temperature adjacent ot surface => TempEffBulkAir
    constexpr int ZoneSupplyAirTemp(3); // supply air temperature of the zone

    constexpr int AltAngStepsForSolReflCalc(10); // Number of steps in altitude angle for solar reflection calc
    constexpr int AzimAngStepsForSolReflCalc(9); // Number of steps in azimuth angle of solar reflection calc

    // Parameters to indicate surface classes
    // Surface Class (FLOOR, WALL, ROOF (incl's CEILING), WINDOW, DOOR, GLASSDOOR,
    // SHADING (includes OVERHANG, WING), DETACHED, INTMASS),
    // TDD:DOME, TDD:DIFFUSER (for tubular daylighting device)
    // (Note: GLASSDOOR and TDD:DIFFUSER get overwritten as WINDOW
    // in SurfaceGeometry.cc, SurfaceWindow%OriginalClass holds the true value)
    // why aren't these sequential

    // Parameters to indicate heat transfer model to use for surface
    extern Array1D_string const HeatTransferModelNames;
    constexpr int HeatTransferModel_NotSet(-1);
    constexpr int HeatTransferModel_None(0); // shading surfaces
    constexpr int HeatTransferModel_CTF(1);
    constexpr int HeatTransferModel_EMPD(2);
    constexpr int HeatTransferModel_CondFD(5);
    constexpr int HeatTransferModel_HAMT(6);
    constexpr int HeatTransferModel_Window5(7);             // original detailed layer-by-layer based on window 4 and window 5
    constexpr int HeatTransferModel_ComplexFenestration(8); // BSDF
    constexpr int HeatTransferModel_TDD(9);                 // tubular daylighting device
    constexpr int HeatTransferModel_Kiva(10);               // Kiva ground calculations
    constexpr int HeatTransferModel_AirBoundaryNoHT(11);    // Construction:AirBoundary - not IRT or interior window

    // Parameters for classification of outside face of surfaces
    constexpr int OutConvClass_WindwardVertWall(101);
    constexpr int OutConvClass_LeewardVertWall(102);
    constexpr int OutConvClass_RoofStable(103);
    constexpr int OutConvClass_RoofUnstable(104);

    // Parameters for adpative convection algorithm's classification of inside face of surfaces
    constexpr int InConvClass_A1_VertWalls(1);           // flow regime A1, vertical walls
    constexpr int InConvClass_A1_StableHoriz(2);         // flow regime A1
    constexpr int InConvClass_A1_UnstableHoriz(3);       // flow regime A1
    constexpr int InConvClass_A1_HeatedFloor(4);         // flow regime A1
    constexpr int InConvClass_A1_ChilledCeil(5);         // flow regime A1
    constexpr int InConvClass_A1_StableTilted(6);        // flow regime A1
    constexpr int InConvClass_A1_UnstableTilted(7);      // flow regime A1
    constexpr int InConvClass_A1_Windows(8);             // flow regime A1
    constexpr int InConvClass_A2_VertWallsNonHeated(9);  // flow regime A2
    constexpr int InConvClass_A2_HeatedVerticalWall(10); // flow regime A2
    constexpr int InConvClass_A2_StableHoriz(11);        // flow regime A2
    constexpr int InConvClass_A2_UnstableHoriz(12);      // flow regime A2
    constexpr int InConvClass_A2_StableTilted(13);       // flow regime A2
    constexpr int InConvClass_A2_UnstableTilted(14);     // flow regime A2
    constexpr int InConvClass_A2_Windows(15);            // flow regime A2
    constexpr int InConvClass_A3_VertWalls(16);          // flow regime A3
    constexpr int InConvClass_A3_StableHoriz(17);        // flow regime A3
    constexpr int InConvClass_A3_UnstableHoriz(18);      // flow regime A3
    constexpr int InConvClass_A3_StableTilted(19);       // flow regime A3
    constexpr int InConvClass_A3_UnstableTilted(20);     // flow regime A3
    constexpr int InConvClass_A3_Windows(21);            // flow regime A3
    constexpr int InConvClass_B_VertWalls(22);           // flow regime B
    constexpr int InConvClass_B_VertWallsNearHeat(23);   // flow regime B
    constexpr int InConvClass_B_StableHoriz(24);         // flow regime B
    constexpr int InConvClass_B_UnstableHoriz(25);       // flow regime B
    constexpr int InConvClass_B_StableTilted(26);        // flow regime B
    constexpr int InConvClass_B_UnstableTilted(27);      // flow regime B
    constexpr int InConvClass_B_Windows(28);             // flow regime B
    constexpr int InConvClass_C_Walls(29);               // flow regime C
    constexpr int InConvClass_C_Ceiling(30);             // flow regime C
    constexpr int InConvClass_C_Floor(31);               // flow regime C
    constexpr int InConvClass_C_Windows(32);             // flow regime C
    constexpr int InConvClass_D_Walls(33);               // flow regime D
    constexpr int InConvClass_D_StableHoriz(34);         // flow regime D
    constexpr int InConvClass_D_UnstableHoriz(35);       // flow regime D
    constexpr int InConvClass_D_StableTilted(36);        // flow regime D
    constexpr int InConvClass_D_UnstableTilted(37);      // flow regime D
    constexpr int InConvClass_D_Windows(38);             // flow regime D
    constexpr int InConvClass_E_AssistFlowWalls(39);     // flow regime E
    constexpr int InConvClass_E_OpposFlowWalls(40);      // flow regime E
    constexpr int InConvClass_E_StableFloor(41);         // flow regime E
    constexpr int InConvClass_E_UnstableFloor(42);       // flow regime E
    constexpr int InConvClass_E_StableCeiling(43);       // flow regime E
    constexpr int InConvClass_E_UnstableCieling(44);     // flow regime E
    constexpr int InConvClass_E_Windows(45);             // flow regime E

    // Parameters for fenestration relative location in zone
    constexpr int InConvWinLoc_NotSet(0);
    constexpr int InConvWinLoc_LowerPartOfExteriorWall(1); // this is a window in the lower part of wall
    constexpr int InConvWinLoc_UpperPartOfExteriorWall(2); // this is a window in the upper part of wall
    constexpr int InConvWinLoc_WindowAboveThis(3);         // this is a wall with window above it
    constexpr int InConvWinLoc_WindowBelowThis(4);         // this is a wall with window below it
    constexpr int InConvWinLoc_LargePartOfExteriorWall(5); // this is a big window taking up most of wall

    // Parameters for window shade status
    constexpr int NoShade(-1);
    constexpr int ShadeOff(0);
    constexpr int IntShadeOn(1); // Interior shade on
    constexpr int SwitchableGlazing(2);
    constexpr int ExtShadeOn(3);  // Exterior shade on
    constexpr int ExtScreenOn(4); // Exterior screen on
    constexpr int IntBlindOn(6);  // Interior blind on
    constexpr int ExtBlindOn(7);  // Exterior blind on
    constexpr int BGShadeOn(8);   // Between-glass shade on
    constexpr int BGBlindOn(9);   // Between-glass blind on
    constexpr int IntShadeConditionallyOff(10);
    constexpr int GlassConditionallyLightened(20);
    constexpr int ExtShadeConditionallyOff(30);
    constexpr int IntBlindConditionallyOff(60);
    constexpr int ExtBlindConditionallyOff(70);

    // WindowShadingControl Shading Types
    constexpr int WSC_ST_NoShade(0);
    constexpr int WSC_ST_InteriorShade(1);
    constexpr int WSC_ST_SwitchableGlazing(2);
    constexpr int WSC_ST_ExteriorShade(3);
    constexpr int WSC_ST_InteriorBlind(4);
    constexpr int WSC_ST_ExteriorBlind(5);
    constexpr int WSC_ST_BetweenGlassShade(6);
    constexpr int WSC_ST_BetweenGlassBlind(7);
    constexpr int WSC_ST_ExteriorScreen(8);

    // WindowShadingControl Control Types
    constexpr int WSCT_AlwaysOn(1);                       // AlwaysOn
    constexpr int WSCT_AlwaysOff(2);                      // AlwaysOff
    constexpr int WSCT_OnIfScheduled(3);                  // OnIfScheduleAllows
    constexpr int WSCT_HiSolar(4);                        // OnIfHighSolarOnWindow
    constexpr int WSCT_HiHorzSolar(5);                    // OnIfHighHorizontalSolar
    constexpr int WSCT_HiOutAirTemp(6);                   // OnIfHighOutsideAirTemp
    constexpr int WSCT_HiZoneAirTemp(7);                  // OnIfHighZoneAirTemp
    constexpr int WSCT_HiZoneCooling(8);                  // OnIfHighZoneCooling
    constexpr int WSCT_HiGlare(9);                        // OnIfHighGlare
    constexpr int WSCT_MeetDaylIlumSetp(10);              // MeetDaylightIlluminanceSetpoint
    constexpr int WSCT_OnNightLoOutTemp_OffDay(11);       // OnNightIfLowOutsideTemp/OffDay
    constexpr int WSCT_OnNightLoInTemp_OffDay(12);        // OnNightIfLowInsideTemp/OffDay
    constexpr int WSCT_OnNightIfHeating_OffDay(13);       // OnNightIfHeating/OffDay
    constexpr int WSCT_OnNightLoOutTemp_OnDayCooling(14); // OnNightIfLowOutsideTemp/OnDayIfCooling
    constexpr int WSCT_OnNightIfHeating_OnDayCooling(15); // OnNightIfHeating/OnDayIfCooling
    constexpr int WSCT_OffNight_OnDay_HiSolarWindow(16);  // OffNight/OnDayIfCoolingAndHighSolarOnWindow
    constexpr int WSCT_OnNight_OnDay_HiSolarWindow(17);   // OnNight/OnDayIfCoolingAndHighSolarOnWindow
    constexpr int WSCT_OnHiOutTemp_HiSolarWindow(18);     // OnIfHighOutsideAirTempAndHighSolarOnWindow
    constexpr int WSCT_OnHiOutTemp_HiHorzSolar(19);       // OnIfHighOutsideAirTempAndHighHorizontalSolar
    constexpr int WSCT_OnHiZoneTemp_HiSolarWindow(20);    // OnIfHighZoneAirTempAndHighSolarOnWindow
    constexpr int WSCT_OnHiZoneTemp_HiHorzSolar(21);      // OnIfHighZoneAirTempAndHighHorizontalSolar

    // WindowShadingControl Slat Angle Control for Blinds
    constexpr int WSC_SAC_FixedSlatAngle(1);
    constexpr int WSC_SAC_ScheduledSlatAngle(2);
    constexpr int WSC_SAC_BlockBeamSolar(3);

    // Parameter for window screens beam reflectance accounting
    constexpr int DoNotModel(0);
    constexpr int ModelAsDirectBeam(1);
    constexpr int ModelAsDiffuse(2);

    // Parameters for window divider type
    constexpr int DividedLite(1);
    constexpr int Suspended(2);

    // Parameters for air flow window source
    constexpr int AirFlowWindow_Source_IndoorAir(1);
    constexpr int AirFlowWindow_Source_OutdoorAir(2);

    // Parameters for air flow window destination
    constexpr int AirFlowWindow_Destination_IndoorAir(1);
    constexpr int AirFlowWindow_Destination_OutdoorAir(2);
    constexpr int AirFlowWindow_Destination_ReturnAir(3);

    // Parameters for air flow window control
    constexpr int AirFlowWindow_ControlType_MaxFlow(1);
    constexpr int AirFlowWindow_ControlType_AlwaysOff(2);
    constexpr int AirFlowWindow_ControlType_Schedule(3);

    // Parameters for window model selection
    constexpr int Window5DetailedModel(100); // indicates original winkelmann window 5 implementation
    constexpr int WindowBSDFModel(101);      // indicates complex fenestration window 6 implementation
    constexpr int WindowEQLModel(102);       // indicates equivalent layer window model implementation

    // Parameters for PierceSurface
    constexpr std::size_t nVerticesBig(20); // Number of convex surface vertices at which to switch to PierceSurface O( log N ) method

    // Surface Window Energy
    extern Array1D<Real64> SurfWinTransSolarEnergy; // Energy of WinTransSolar [J]
    extern Array1D<Real64> SurfWinBmSolarEnergy;    // Energy of WinBmSolar [J]

    extern Array1D<Real64> SurfWinBmBmSolarEnergy;  // Beam-to-beam energy of WinBmSolar [J]
    extern Array1D<Real64> SurfWinBmDifSolarEnergy; // Beam-to-diffuse energy of WinBmSolar [J]

    extern Array1D<Real64> SurfWinDifSolarEnergy;             // Energy of WinDifSolar [J]
    extern Array1D<Real64> SurfWinHeatGainRepEnergy;          // Energy of WinHeatGainRep [J]
    extern Array1D<Real64> SurfWinHeatLossRepEnergy;          // Energy of WinHeatLossRep [J]
    extern Array1D<Real64> SurfWinShadingAbsorbedSolarEnergy; // Energy of WinShadingAbsorbedSolar [J]
    extern Array1D<Real64> SurfWinGapConvHtFlowRepEnergy;     // Energy of WinGapConvHtFlowRep [J]
    extern Array1D<Real64> SurfWinHeatTransferRepEnergy;      // Energy of WinHeatTransfer [J]

    // SurfaceWindow Struct
    extern Array1D<Real64> SurfWinIRfromParentZone;
    extern Array1D<Real64> SurfWinFrameQRadOutAbs;
    extern Array1D<Real64> SurfWinFrameQRadInAbs;
    extern Array1D<Real64> SurfWinDividerQRadOutAbs;
    extern Array1D<Real64> SurfWinDividerQRadInAbs;
    extern Array1D<Real64> SurfWinExtBeamAbsByShade;       // Exterior beam solar absorbed by window shade (W/m2)
    extern Array1D<Real64> SurfWinExtDiffAbsByShade;       // Exterior diffuse solar absorbed by window shade (W/m2)
    extern Array1D<Real64> SurfWinIntBeamAbsByShade;       // Interior beam solar absorbed by window shade (W/m2)
    extern Array1D<Real64> SurfWinIntSWAbsByShade;         // Interior diffuse solar plus short-wave from lights absorbed by window shade (W/m2)
    extern Array1D<Real64> SurfWinInitialDifSolAbsByShade; // Initial diffuse solar from ext and int windows absorbed by window shade (W/m2)
    extern Array1D<Real64> SurfWinIntLWAbsByShade;         // Interior long-wave from zone lights and equipment absorbed by window shade (W/m2)
    extern Array1D<Real64> SurfWinConvHeatFlowNatural;     // Convective heat flow from gap between glass and interior shade or blind (W)
    extern Array1D<Real64> SurfWinConvHeatGainToZoneAir;   // Convective heat gain to zone air from window gap airflow (W)
    extern Array1D<Real64> SurfWinRetHeatGainToZoneAir;    // Convective heat gain to return air sent to zone [W]
    extern Array1D<Real64> SurfWinDividerHeatGain;
    extern Array1D<Real64> SurfWinBlTsolBmBm;                 // Time-step value of blind beam-beam solar transmittance (-)
    extern Array1D<Real64> SurfWinBlTsolBmDif;                // Time-step value of blind beam-diffuse solar transmittance (-)
    extern Array1D<Real64> SurfWinBlTsolDifDif;               // Time-step value of blind diffuse-diffuse solar transmittance (-)
    extern Array1D<Real64> SurfWinBlGlSysTsolBmBm;            // Time-step value of blind/glass system beam-beam solar transmittance (-)
    extern Array1D<Real64> SurfWinBlGlSysTsolDifDif;          // Time-step value of blind/glass system diffuse-diffuse solar transmittance (-)
    extern Array1D<Real64> SurfWinScTsolBmBm;                 // Time-step value of screen beam-beam solar transmittance (-)
    extern Array1D<Real64> SurfWinScTsolBmDif;                // Time-step value of screen beam-diffuse solar transmittance (-)
    extern Array1D<Real64> SurfWinScTsolDifDif;               // Time-step value of screen diffuse-diffuse solar transmittance (-)
    extern Array1D<Real64> SurfWinScGlSysTsolBmBm;            // Time-step value of screen/glass system beam-beam solar transmittance (-)
    extern Array1D<Real64> SurfWinScGlSysTsolDifDif;          // Time-step value of screen/glass system diffuse-diffuse solar transmittance (-)
    extern Array1D<Real64> SurfWinGlTsolBmBm;                 // Time-step value of glass beam-beam solar transmittance (-)
    extern Array1D<Real64> SurfWinGlTsolBmDif;                // Time-step value of glass beam-diffuse solar transmittance (-)
    extern Array1D<Real64> SurfWinGlTsolDifDif;               // Time-step value of glass diffuse-diffuse solar transmittance (-)
    extern Array1D<Real64> SurfWinBmSolTransThruIntWinRep;    // Beam solar transmitted through interior window [W]
    extern Array1D<Real64> SurfWinBmSolAbsdOutsReveal;        // Multiplied by BeamSolarRad, gives beam solar absorbed by outside reveal surfaces (m2)
    extern Array1D<Real64> SurfWinBmSolRefldOutsRevealReport; // Beam solar reflected by outside reveal surfaces, for reporting (m2)
    extern Array1D<Real64> SurfWinBmSolAbsdInsReveal;         // Multiplied by BeamSolarRad, gives beam solar absorbed by inside reveal surfaces (m2)
    extern Array1D<Real64> SurfWinBmSolRefldInsReveal;        // Multiplied by BeamSolarRad, gives beam solar reflected by inside reveal surfaces (m2)
    extern Array1D<Real64> SurfWinBmSolRefldInsRevealReport;  // Beam solar reflected by inside reveal surfaces, for reporting (W)
    extern Array1D<Real64> SurfWinOutsRevealDiffOntoGlazing;  // Multiplied by BeamSolarRad, gives diffuse from beam reflection from outside reveal
                                                              // that is incident on the glazing per m2 of glazing (-)
    extern Array1D<Real64> SurfWinInsRevealDiffOntoGlazing; // Multiplied by BeamSolarRad, gives diffuse from beam reflection from inside reveal that
                                                            // is incident on the glazing per m2 of glazing (-)
    extern Array1D<Real64> SurfWinInsRevealDiffIntoZone;    // Multiplied by BeamSolarRad, gives diffuse from beam reflection from inside reveal that
                                                            // goes into zone directly or reflected from glazing (m2)
    extern Array1D<Real64> SurfWinOutsRevealDiffOntoFrame;  // Multiplied by BeamSolarRad, gives diffuse from beam reflection from outside reveal that
                                                            // is incident on the outside of the frame per m2 of frame (-)
    extern Array1D<Real64>
        SurfWinInsRevealDiffOntoFrame; // Multiplied by BeamSolarRad, gives diffuse from beam reflection from inside reveal that is incident on the
                                       // outside of the frame per m2 of frame (-) for debugging CR 7596. TH 5/26/2009
    extern Array1D<Real64>
        SurfWinInsRevealDiffOntoGlazingReport; // Diffuse solar from beam reflection from inside reveal that is incident on the glazing (W)
    extern Array1D<Real64> SurfWinInsRevealDiffIntoZoneReport;   // Diffuse from beam reflection from inside reveal that goes into zone directly or
                                                                 // reflected from glazing (W)
    extern Array1D<Real64> SurfWinInsRevealDiffOntoFrameReport;  // Diffuse from beam reflection from inside reveal that is incident on the frame (W)
    extern Array1D<Real64> SurfWinBmSolAbsdInsRevealReport;      // Beam solar absorbed by inside reveal (W)  energy
    extern Array1D<Real64> SurfWinBmSolTransThruIntWinRepEnergy; // energy of BmSolTransThruIntWinRep [J]
    extern Array1D<Real64> SurfWinBmSolRefldOutsRevealRepEnergy; // energy of BmSolRefldOutsRevealReport [J]
    extern Array1D<Real64> SurfWinBmSolRefldInsRevealRepEnergy;  // energy of BmSolRefldInsRevealReport [J]
    extern Array1D<Real64> SurfWinProfileAngHor;                 // Horizontal beam solar profile angle (degrees)
    extern Array1D<Real64> SurfWinProfileAngVert;                // Vertical beam solar profile angle (degrees)

    extern Array1D<int> SurfWinShadingFlag;         // -1: window has no shading device
    extern Array1D<bool> SurfWinShadingFlagEMSOn;   // EMS control flag, true if EMS is controlling ShadingFlag with ShadingFlagEMSValue
    extern Array1D<int> SurfWinShadingFlagEMSValue; // EMS control value for Shading Flag
    extern Array1D<int>
        SurfWinStormWinFlag; // -1: Storm window not applicable; 0: Window has storm window but it is off 1: Window has storm window and it is on
    extern Array1D<int> SurfWinStormWinFlagPrevDay; // Previous time step value of StormWinFlag
    extern Array1D<Real64>
        SurfWinFracTimeShadingDeviceOn; // For a single time step, = 0.0 if no shading device or shading device is off = 1.0 if shading device is on;
                                        // For time intervals longer than a time step, = fraction of time that shading device is on.
    extern Array1D<int> SurfWinExtIntShadePrevTS;     // 1 if exterior or interior blind or shade in place previous time step;0 otherwise
    extern Array1D<bool> SurfWinHasShadeOrBlindLayer; // mark as true if the window construction has a shade or a blind layer
    extern Array1D<bool> SurfWinSurfDayLightInit;     // surface has been initialized for following 5 arrays
    extern Array1D<int> SurfWinDaylFacPoint;          // Pointer to daylight factors for the window
    extern Array1D<Real64> SurfWinVisTransSelected;   // Window vis trans at normal incidence selected for use in dayltg calculation
    extern Array1D<Real64> SurfWinSwitchingFactor;    // Window switching factor (0.0 = unswitched; 1.0 = fully switched)
    extern Array1D<Real64> SurfWinTheta;              // Azimuth of window normal (rad)
    extern Array1D<Real64> SurfWinPhi;                // Altitude of window normal (rad)
    extern Array1D<Real64>
        SurfWinRhoCeilingWall;                  // Average interior reflectance seen by light moving up across horizontal plane thru center of window
    extern Array1D<Real64> SurfWinRhoFloorWall; // Same as above, but for light moving down
    extern Array1D<Real64> SurfWinFractionUpgoing; // Fraction light entering window that goes upward
    extern Array1D<Real64>
        SurfWinVisTransRatio; // For windows with switchable glazing, ratio of normal transmittance in switched state to that in unswitched state
    extern Array1D<Real64> SurfWinFrameArea;                  // Frame projected area (m2)
    extern Array1D<Real64> SurfWinFrameConductance;           // Frame conductance [no air films] (W/m2-K)
    extern Array1D<Real64> SurfWinFrameSolAbsorp;             // Frame solar absorptance (assumed same inside and outside)
    extern Array1D<Real64> SurfWinFrameVisAbsorp;             // Frame visible absorptance (assumed same inside and outside)
    extern Array1D<Real64> SurfWinFrameEmis;                  // Frame thermal emissivity (thermal absorptance) (assumed same inside and outside)
    extern Array1D<Real64> SurfWinFrEdgeToCenterGlCondRatio;  // Ratio of frame edge of glass conductance (without air films) to center of glass
                                                              // conductance (without air films)
    extern Array1D<Real64> SurfWinFrameEdgeArea;              // Area of glass near frame (m2)
    extern Array1D<Real64> SurfWinFrameTempSurfIn;            // Frame inside surface temperature (C)
    extern Array1D<Real64> SurfWinFrameTempSurfInOld;         // Previous value of frame inside surface temperature (C)
    extern Array1D<Real64> SurfWinFrameTempSurfOut;           // Frame outside surface temperature (C)
    extern Array1D<Real64> SurfWinProjCorrFrOut;              // Correction factor to absorbed radiation due to frame outside projection
    extern Array1D<Real64> SurfWinProjCorrFrIn;               // Correction factor to absorbed radiation due to frame inside projection
    extern Array1D<int> SurfWinDividerType;                   // Divider type (1=DividedLite, 2=Suspended (between-pane))
    extern Array1D<Real64> SurfWinDividerArea;                // Divider projected area (m2)
    extern Array1D<Real64> SurfWinDividerConductance;         // Divider conductance [no air films] (W/m2-K)
    extern Array1D<Real64> SurfWinDividerSolAbsorp;           // Divider solar absorptance (assumed same inside and outside)
    extern Array1D<Real64> SurfWinDividerVisAbsorp;           // Divider visible absorptance (assumed same inside and outside)
    extern Array1D<Real64> SurfWinDividerEmis;                // Divider thermal emissivity (thermal absorptance) (assumed same inside and outside)
    extern Array1D<Real64> SurfWinDivEdgeToCenterGlCondRatio; // Ratio of divider edge of glass conductance (without air films) to center of glass
                                                              // conductance (without air films)
    extern Array1D<Real64> SurfWinDividerEdgeArea;            // Area of glass near dividers (m2)
    extern Array1D<Real64> SurfWinDividerTempSurfIn;          // Divider inside surface temperature (C)
    extern Array1D<Real64> SurfWinDividerTempSurfInOld;       // Previous value of divider inside surface temperature (C)
    extern Array1D<Real64> SurfWinDividerTempSurfOut;         // Divider outside surface temperature (C)
    extern Array1D<Real64> SurfWinProjCorrDivOut;             // Correction factor to absorbed radiation due to divider outside projection
    extern Array1D<Real64> SurfWinProjCorrDivIn;              // Correction factor to absorbed radiation due to divider inside projection
    extern Array1D<Real64> SurfWinGlazedFrac;                 // (Glazed area)/(Glazed area + divider area)
    extern Array1D<Real64> SurfWinCenterGlArea;               // Center of glass area (m2); area of glass where 1-D conduction dominates
    extern Array1D<Real64> SurfWinEdgeGlCorrFac;       // Correction factor to center-of-glass conductance to account for 2-D glass conduction thermal
                                                       // bridging effects near frame and divider
    extern Array1D<SurfaceClass> SurfWinOriginalClass; // 0 or if entered originally as:
    extern Array1D<Real64> SurfWinShadeAbsFacFace1;    // Fraction of short-wave radiation incident that is absorbed by face 1 when total absorbed
                                                       // radiation is apportioned to the two faces
    extern Array1D<Real64> SurfWinShadeAbsFacFace2;    // Fraction of short-wave radiation incident that is absorbed by face 2 when total absorbed
                                                       // radiation is apportioned to the two faces
    extern Array1D<Real64>
        SurfWinConvCoeffWithShade; // Convection coefficient from glass or shade to gap air when interior or exterior shade is present (W/m2-K)
    extern Array1D<Real64> SurfWinOtherConvHeatGain;   // other convective = total conv - standard model prediction for EQL window model (W)
    extern Array1D<int> SurfWinBlindNumber;            // Blind number for a window with a blind
    extern Array1D<Real64> SurfWinEffInsSurfTemp;      // Effective inside surface temperature for window with interior blind or shade; combination of
                                                       // shade/blind and glass temperatures (C)
    extern Array1D<bool> SurfWinMovableSlats;          // True if window has a blind with movable slats
    extern Array1D<Real64> SurfWinSlatAngThisTS;       // Slat angle this time step for window with blind on (radians)
    extern Array1D<Real64> SurfWinSlatAngThisTSDeg;    // Slat angle this time step for window with blind on (deg)
    extern Array1D<bool> SurfWinSlatAngThisTSDegEMSon; // flag that indicate EMS system is actuating SlatAngThisTSDeg
    extern Array1D<Real64> SurfWinSlatAngThisTSDegEMSValue; // value that EMS sets for slat angle in degrees
    extern Array1D<bool> SurfWinSlatsBlockBeam;             // True if blind slats block incident beam solar
    extern Array1D<Real64>
        SurfWinBlindAirFlowPermeability; // Blind air-flow permeability for calculation of convective flow in gap between blind and glass
    extern Array1D<Real64> SurfWinTotGlazingThickness;      // Total glazing thickness from outside of outer glass to inside of inner glass (m)
    extern Array1D<Real64> SurfWinTanProfileAngHor;         // Tangent of horizontal profile angle
    extern Array1D<Real64> SurfWinTanProfileAngVert;        // Tangent of vertical profile angle
    extern Array1D<Real64> SurfWinInsideSillDepth;          // Depth of inside sill (m)
    extern Array1D<Real64> SurfWinInsideReveal;             // Depth of inside reveal (m)
    extern Array1D<Real64> SurfWinInsideSillSolAbs;         // Solar absorptance of inside sill
    extern Array1D<Real64> SurfWinInsideRevealSolAbs;       // Solar absorptance of inside reveal
    extern Array1D<Real64> SurfWinOutsideRevealSolAbs;      // Solar absorptance of outside reveal
    extern Array1D<int> SurfWinScreenNumber;                // Screen number for a window with a screen (do not confuse with material number)
    extern Array1D<int> SurfWinAirflowSource;               // Source of gap airflow (INSIDEAIR, OUTSIDEAIR, etc.)
    extern Array1D<int> SurfWinAirflowDestination;          // Destination of gap airflow (INSIDEAIR, OUTSIDEAIR, etc.)
    extern Array1D<int> SurfWinAirflowReturnNodePtr;        // Return node pointer for destination = ReturnAir
    extern Array1D<Real64> SurfWinMaxAirflow;               // Maximum gap airflow (m3/s per m of glazing width)
    extern Array1D<int> SurfWinAirflowControlType;          // Gap airflow control type (ALWAYSONATMAXFLOW, etc.)
    extern Array1D<bool> SurfWinAirflowHasSchedule;         // True if gap airflow is scheduled
    extern Array1D<int> SurfWinAirflowSchedulePtr;          // Gap airflow schedule pointer
    extern Array1D<Real64> SurfWinAirflowThisTS;            // Gap airflow this timestep (m3/s per m of glazing width)
    extern Array1D<Real64> SurfWinTAirflowGapOutlet;        // Temperature of air leaving airflow gap between glass panes (C)
    extern Array1D<int> SurfWinWindowCalcIterationsRep;     // Number of iterations in window heat balance calculation
    extern Array1D<Real64> SurfWinVentingOpenFactorMultRep; // Window/door opening modulation multiplier on venting open factor, for reporting
    extern Array1D<Real64> SurfWinInsideTempForVentingRep;  // Inside air temp used to control window/door venting, for reporting (C)
    extern Array1D<Real64> SurfWinVentingAvailabilityRep;   // Venting availability schedule value (0.0/1.0 = no venting allowed/not allowed)
    extern Array1D<Real64> SurfWinSkyGndSolarInc; // Incident diffuse solar from ground-reflected sky radiation; used for Complex Fen; if CalcSolRefl
                                                  // is true, accounts for shadowing of ground by building and obstructions [W/m2]
    extern Array1D<Real64> SurfWinBmGndSolarInc;  // Incident diffuse solar from ground-reflected beam radiation; used for Complex Fen; if CalcSolRefl
                                                  // is true, accounts for shadowing of ground by building and obstructions [W/m2]
    extern Array1D<Real64> SurfWinLightWellEff;   // Light well efficiency (multiplier on exterior window vis trans due to light well losses)
    extern Array1D<bool> SurfWinSolarDiffusing;   // True if exterior window with a construction that contains a diffusing glass layer
    extern Array1D<Real64> SurfWinFrameHeatGain;
    extern Array1D<Real64> SurfWinFrameHeatLoss;
    extern Array1D<Real64> SurfWinDividerHeatLoss;
    extern Array1D<Real64> SurfWinTCLayerTemp;     // The temperature of the thermochromic layer of the window
    extern Array1D<Real64> SurfWinSpecTemp;        // The specification temperature of the TC layer glass Added for W6 integration June 2010
    extern Array1D<Real64> SurfWinWindowModelType; // if set to WindowBSDFModel, then uses BSDF methods
    extern Array1D<Real64> SurfWinTDDPipeNum;      // Tubular daylighting device pipe number for TDD domes and diffusers

    extern bool AnyHeatBalanceInsideSourceTerm;  // True if any SurfaceProperty:HeatBalanceSourceTerm inside face used
    extern bool AnyHeatBalanceOutsideSourceTerm; // True if any SurfaceProperty:HeatBalanceSourceTerm outside face used

    // SUBROUTINE SPECIFICATIONS FOR MODULE DataSurfaces:

    // Types

    // Y Slab for Surface2D for PierceSurface support of Nonconvex and Many-Vertex Surfaces
    struct Surface2DSlab
    {

    public: // Types
        using Vertex = ObjexxFCL::Vector2<Real64>;
        using Vertices = ObjexxFCL::Array1D<Vertex>;
        using Edge = Vertices::size_type; // The Surface2D vertex and edge index
        using EdgeXY = Real64;            // The edge x/y inverse slope
        using Edges = std::vector<Edge>;
        using EdgesXY = std::vector<EdgeXY>;

    public: // Creation
            // Constructor
        Surface2DSlab(Real64 const yl, Real64 const yu) : xl(0.0), xu(0.0), yl(yl), yu(yu)
        {
        }

    public:              // Data
        Real64 xl, xu;   // Lower and upper x coordinates of slab bounding box
        Real64 yl, yu;   // Lower and upper y coordinates of slab
        Edges edges;     // Left-to-right ordered edges crossing the slab
        EdgesXY edgesXY; // Edge x/y inverse slopes

    }; // Surface2DSlab

    // Projected 2D Surface Representation for Fast Computational Geometry Operations
    struct Surface2D
    {

    public: // Types
        using Vector2D = Vector2<Real64>;
        using Edge = Vector2D;
        using Vertices = Array1D<Vector2D>;
        using Vectors = Array1D<Vector2D>;
        using Edges = Vectors;
        using Slab = Surface2DSlab;
        using Slabs = std::vector<Surface2DSlab>;
        using SlabYs = std::vector<Real64>;
        using size_type = Vertices::size_type;

    public: // Creation
        // Default constructor
        Surface2D()
        {
        }

        // Constructor
        Surface2D(ShapeCat const shapeCat, int const axis, Vertices const &v, Vector2D const &vl, Vector2D const &vu);

    public: // Predicates
            // Bounding box contains a point?
        bool bb_contains(Vector2D const &v) const
        {
            return (vl.x <= v.x) && (v.x <= vu.x) && (vl.y <= v.y) && (v.y <= vu.y);
        }

    public: // Comparison
            // Equality
        friend bool operator==(Surface2D const &a, Surface2D const &b)
        {
            auto const &v1 = a.vertices;
            auto const &v2 = b.vertices;
            return eq(v1, v2);
        }

        // Inequality
        friend bool operator!=(Surface2D const &a, Surface2D const &b)
        {
            return !(a == b);
        }

    public:                                              // Data
        int axis = 0;                                    // Axis of projection (0=x, 1=y, 2=z)
        Vertices vertices;                               // Vertices
        Vector2D vl = Vector2D(0.0), vu = Vector2D(0.0); // Bounding box lower and upper corner vertices
        Vectors edges;                                   // Edge vectors around the vertices
        Real64 s1 = 0.0, s3 = 0.0;                       // Rectangle side widths squared
        SlabYs slabYs;                                   // Y coordinates of slabs
        Slabs slabs;                                     // Y slice slabs for fast nonconvex and many vertex intersections

    }; // Surface2D

    struct SurfaceData
    {

        // Types
        using Vertices = Array1D<Vector>;
        using Plane = Vector4<Real64>;

        // Members
        std::string Name;                 // User supplied name of the surface (must be unique)
        int Construction;                 // Pointer to the construction in the Construct derived type
        bool EMSConstructionOverrideON;   // if true, EMS is calling to override the construction value
        int EMSConstructionOverrideValue; // pointer value to use for Construction when overridden
        int ConstructionStoredInputValue; // holds the original value for Construction per surface input
        SurfaceClass Class;
        // Geometry related parameters
        SurfaceShape Shape; // Surface shape (Triangle=1,Quadrilateral=2,Rectangle=3,
        //                Rectangular Window/Door=4,Rectangular Overhang=5,
        //                Rectangular Left Fin=6,Rectangular Right Fin=7,
        //                Triangular Window=8)
        int Sides;                // Number of side/vertices for this surface (based on Shape)
        Real64 Area;              // Surface area of the surface (less any subsurfaces) {m2}
        Real64 GrossArea;         // Surface area of the surface (including subsurfaces) {m2}
        Real64 NetAreaShadowCalc; // Area of a wall/floor/ceiling less subsurfaces assuming
        //  all windows, if present, have unity multiplier.
        // Wall/floor/ceiling/roof areas that include windows include
        //  frame (unity) areas.
        // Areas of Windows including divider (unity) area.
        // These areas are used in shadowing / sunlit area calculations.
        Real64 Perimeter; // Perimeter length of the surface {m}
        Real64 Azimuth;   // Direction the surface outward normal faces (degrees) or FACING
        Real64 Height;    // Height of the surface (m)
        Real64 Reveal;    // Depth of the window reveal (m) if this surface is a window
        Real64 Tilt;      // Angle (deg) between the ground outward normal and the surface outward normal
        Real64 Width;     // Width of the surface (m)
        // Boundary conditions and interconnections
        bool HeatTransSurf;                // True if surface is a heat transfer surface,
        int OutsideHeatSourceTermSchedule; // Pointer to the schedule of additional source of heat flux rate applied to the outside surface
        int InsideHeatSourceTermSchedule;  // Pointer to the schedule of additional source of heat flux rate applied to the inside surface
        // False if a (detached) shadowing (sub)surface
        int HeatTransferAlgorithm; // used for surface-specific heat transfer algorithm.
        std::string BaseSurfName;  // Name of BaseSurf
        int BaseSurf;              // "Base surface" for this surface.  Applies mainly to subsurfaces
        // in which case it points back to the base surface number.
        // Equals 0 for detached shading.
        // BaseSurf equals surface number for all other surfaces.
        int NumSubSurfaces;   // Number of subsurfaces this surface has (doors/windows)
        std::string ZoneName; // User supplied name of the Zone
        int Zone;             // Interior environment or zone the surface is a part of
        // Note that though attached shading surfaces are part of a zone, this
        // value is 0 there to facilitate using them as detached surfaces (more
        // accurate shading.
        std::string ExtBoundCondName; // Name for the Outside Environment Object
        int ExtBoundCond;             // For an "interzone" surface, this is the adjacent surface number.
        // for an internal/adiabatic surface this is the current surface number.
        // Otherwise, 0=external environment, -1=ground,
        // -2=other side coefficients (OSC--won't always use CTFs)
        // -3=other side conditions model
        // During input, interim values of UnreconciledZoneSurface ("Surface") and
        // UnenteredAdjacentZoneSurface ("Zone") are used until reconciled.
        int LowTempErrCount;
        int HighTempErrCount;
        bool ExtSolar; // True if the "outside" of the surface is exposed to solar
        bool ExtWind;  // True if the "outside" of the surface is exposed to wind
        // Heat transfer coefficients
        int IntConvCoeff; // Interior Convection Coefficient pointer (different data structure)
        // when being overridden
        bool EMSOverrideIntConvCoef;   // if true, EMS is calling to override interior convection coefficeint
        Real64 EMSValueForIntConvCoef; // Value EMS is calling to use for interior convection coefficient [W/m2-K]
        int ExtConvCoeff;              // Exterior Convection Coefficient pointer (different data structure)
        // when being overridden
        bool EMSOverrideExtConvCoef;   // if true, EMS is calling to override exterior convection coefficeint
        Real64 EMSValueForExtConvCoef; // Value EMS is calling to use for exterior convection coefficient [W/m2-K]
        Real64 ViewFactorGround;       // View factor to the ground from the exterior of the surface
        //   for diffuse solar radiation
        Real64 ViewFactorSky; // View factor to the sky from the exterior of the surface
        //   for diffuse solar radiation
        Real64 ViewFactorGroundIR; // View factor to the ground and shadowing surfaces from the
        //    exterior of the surface for IR radiation
        Real64 ViewFactorSkyIR; // View factor to the sky from the exterior of the surface for IR radiation
        // Special/optional other side coefficients (OSC)
        int OSCPtr;  // Pointer to OSC data structure
        int OSCMPtr; // "Pointer" to OSCM data structure (other side conditions from a model)
        // Optional parameters specific to shadowing surfaces and subsurfaces (detached shading, overhangs, wings, etc.)
        int SchedShadowSurfIndex;   // Schedule for a shadowing (sub)surface
        bool ShadowSurfSchedVaries; // true if the scheduling (transmittance) on a shading surface varies.
        bool ShadowingSurf;         // True if a surface is a shadowing surface
        bool IsTransparent;         // True if the schedule values are always 1.0 (or the minimum is 1.0)
        Real64 SchedMinValue;       // Schedule minimum value.
        // Optional parameters specific to solar reflection from surfaces
        Real64 ShadowSurfDiffuseSolRefl;    // Diffuse solar reflectance of opaque portion
        Real64 ShadowSurfDiffuseVisRefl;    // Diffuse visible reflectance of opaque portion
        Real64 ShadowSurfGlazingFrac;       // Glazing fraction
        int ShadowSurfGlazingConstruct;     // Glazing construction number
        bool ShadowSurfPossibleObstruction; // True if a surface can be an exterior obstruction
        bool ShadowSurfPossibleReflector;   // True if a surface can be an exterior reflector, not used!
        int ShadowSurfRecSurfNum;           // Receiving surface number
        // Optional movable insulation parameters
        int MaterialMovInsulExt;       // Pointer to the material used for exterior movable insulation
        int MaterialMovInsulInt;       // Pointer to the material used for interior movable insulation
        int SchedMovInsulExt;          // Schedule for exterior movable insulation
        int SchedMovInsulInt;          // Schedule for interior movable insulation
        bool MovInsulIntPresent;       // True when movable insulation is present
        bool MovInsulIntPresentPrevTS; // True when movable insulation was present during the previous time step
        // Vertices
        Array1D<Vector> NewVertex;
        Vertices Vertex; // Surface Vertices are represented by Number of Sides and Vector (type)
        Vector Centroid; // computed centroid (also known as center of mass or surface balance point)
        Vector lcsx;
        Vector lcsy;
        Vector lcsz;
        Vector NewellAreaVector;
        Vector NewellSurfaceNormalVector; // same as OutNormVec in vector notation
        Array1D<Real64> OutNormVec;       // Direction cosines (outward normal vector) for surface
        Real64 SinAzim;                   // Sine of surface azimuth angle
        Real64 CosAzim;                   // Cosine of surface azimuth angle
        Real64 SinTilt;                   // Sine of surface tilt angle
        Real64 CosTilt;                   // Cosine of surface tilt angle
        bool IsConvex;                    // true if the surface is convex.
        bool IsDegenerate;                // true if the surface is degenerate.
        bool VerticesProcessed;           // true if vertices have been processed (only used for base surfaces)
        Real64 XShift;                    // relative coordinate shift data - used by child subsurfaces
        Real64 YShift;                    // relative coordinate shift data - used by child subsurfaces
        // Precomputed parameters for PierceSurface performance
        ShapeCat shapeCat;   // Shape category
        Plane plane;         // Plane
        Surface2D surface2d; // 2D projected surface for efficient intersection testing
        // Window Parameters (when surface is Window)
        int activeWindowShadingControl;            // Active window shading control (windows only)
        std::vector<int> windowShadingControlList; // List of possible window shading controls
        bool HasShadeControl;                      // True if the surface is listed in a WindowShadingControl object
        int activeShadedConstruction;              // The currently active shaded construction (windows only)
        std::vector<int> shadedConstructionList;   // List of shaded constructions that correspond with window shading controls (windows only - same
                                                   // indexes as windowShadingControlList)
        int StormWinConstruction;                  // Construction with storm window (windows only)
        int activeStormWinShadedConstruction;      // The currently active shaded construction with storm window (windows only)
        std::vector<int> shadedStormWinConstructionList; // List of shaded constructions with storm window that correspond with window shading
                                                         // controls (windows only - same indexes as windowShadingControlList)
        int FrameDivider;                                // Pointer to frame and divider information (windows only)
        Real64 Multiplier;                               // Multiplies glazed area, frame area and divider area (windows only)
        // Daylighting pointers
        int Shelf;   // Pointer to daylighting shelf
        int TAirRef; // Flag for reference air temperature
        // ZoneMeanAirTemp   = 1 = mean air temperature or MAT => for mixing air model with all convection algos
        // except inlet-dependent algo
        // AdjacentAirTemp   = 2 = adjacent air temperature or TempEffBulkAir => for nodal or zonal air model
        // with all convection algos except inlet-dependent algo
        // ZoneSupplyAirTemp = 3 = supply air temperature => for mixing air model with inlet-dependent algo
        // Default value is 'ZoneMeanAirTemp' and value for each particular surface will be changed only if
        // the inlet-dependent convection algorithm and/or nodal and zonal air models are used.
        Real64 OutDryBulbTemp;                 // Surface outside dry bulb air temperature, for surface heat balance (C)
        bool OutDryBulbTempEMSOverrideOn;      // if true, EMS is calling to override the surface's outdoor air temp
        Real64 OutDryBulbTempEMSOverrideValue; // value to use for EMS override of outdoor air drybulb temp (C)
        Real64 OutWetBulbTemp;                 // Surface outside wet bulb air temperature, for surface heat balance (C)
        bool OutWetBulbTempEMSOverrideOn;      // if true, EMS is calling to override the surface's outdoor wetbulb
        Real64 OutWetBulbTempEMSOverrideValue; // value to use for EMS override of outdoor air wetbulb temp (C)
        Real64 WindSpeed;                      // Surface outside wind speed, for surface heat balance (m/s)
        bool WindSpeedEMSOverrideOn;
        Real64 WindSpeedEMSOverrideValue;
        bool ViewFactorGroundEMSOverrideOn;      // if true, EMS is calling to override the surface's view factor to ground
        Real64 ViewFactorGroundEMSOverrideValue; // value to use for EMS override of the surface's view factor to ground

        Real64 WindDir;                    // Surface outside wind direction, for surface heat balance and ventilation(degree)
        bool WindDirEMSOverrideOn;         // if true, EMS is calling to override the surface's outside wind direction
        Real64 WindDirEMSOverrideValue;    // value to use for EMS override of the surface's outside wind speed
        bool SchedExternalShadingFrac;     // true if the external shading is scheduled or calculated externally to be imported
        int ExternalShadingSchInd;         // Schedule for a the external shading
        bool HasSurroundingSurfProperties; // true if surrounding surfaces properties are listed for an external surface
        int SurroundingSurfacesNum;        // Index of a surrounding surfaces list (defined in SurfaceProperties::SurroundingSurfaces)
        bool HasLinkedOutAirNode;          // true if an OutdoorAir::Node is linked to the surface
        int LinkedOutAirNode;              // Index of the an OutdoorAir:Node

        int PenumbraID; // Surface ID in penumbra

        std::string UNomWOFilm; // Nominal U Value without films stored as string
        std::string UNomFilm;   // Nominal U Value with films stored as string
        bool ExtEcoRoof;        // True if the top outside construction material is of type Eco Roof
        bool ExtCavityPresent;  // true if there is an exterior vented cavity on surface
        int ExtCavNum;          // index for this surface in ExtVentedCavity structure (if any)
        bool IsPV;              // true if this is a photovoltaic surface (dxf output)
        bool IsICS;             // true if this is an ICS collector
        bool IsPool;            // true if this is a pool
        int ICSPtr;             // Index to ICS collector
        // TH added 3/26/2010
        bool MirroredSurf; // True if it is a mirrored surface
        // additional attributes for convection correlations
        int IntConvClassification;       // current classification for inside face air flow regime and surface orientation
        int IntConvHcModelEq;            // current convection model for inside face
        int IntConvHcUserCurveIndex;     // current index to user convection model if used
        int OutConvClassification;       // current classification for outside face wind regime and convection orientation
        int OutConvHfModelEq;            // current convection model for forced convection at outside face
        int OutConvHfUserCurveIndex;     // current index to user forced convection model if used
        int OutConvHnModelEq;            // current Convection model for natural convection at outside face
        int OutConvHnUserCurveIndex;     // current index to user natural convection model if used
        Real64 OutConvFaceArea;          // area of larger building envelope facade that surface is a part of
        Real64 OutConvFacePerimeter;     // perimeter of larger building envelope facade that surface is a part of
        Real64 OutConvFaceHeight;        // height of larger building envelope facade that surface is a part of
        Real64 IntConvZoneWallHeight;    // [m] height of larger inside building wall element that surface is a part of
        Real64 IntConvZonePerimLength;   // [m] length of perimeter zone's exterior wall
        Real64 IntConvZoneHorizHydrDiam; // [m] hydraulic diameter, usually 4 times the zone floor area div by perimeter
        Real64 IntConvWindowWallRatio;   // [-] area of windows over area of exterior wall for zone
        int IntConvWindowLocation;       // relative location of window in zone for interior Hc models
        bool IntConvSurfGetsRadiantHeat;
        bool IntConvSurfHasActiveInIt;
        bool IsRadSurfOrVentSlabOrPool; // surface cannot be part of both a radiant surface & ventilated slab group
        // LG added 1/6/12
        Real64 GenericContam; // [ppm] Surface generic contaminant as a storage term for
        // Air boundaries
        int SolarEnclIndex;     // Pointer to solar enclosure this surface belongs to
        int SolarEnclSurfIndex; //  Pointer to solar enclosure surface data, ZoneSolarInfo(n).SurfacePtr(RadEnclSurfIndex) points to this surface
        bool IsAirBoundarySurf; // True if surface is an air boundary surface (Construction:AirBoundary),

        std::vector<int> DisabledShadowingZoneList; // Array of all disabled shadowing zone number to the current surface
                                                    // the surface diffusion model

        // Default Constructor
        SurfaceData()
            : Construction(0), EMSConstructionOverrideON(false), EMSConstructionOverrideValue(0), ConstructionStoredInputValue(0),
              Class(SurfaceClass::None), Shape(SurfaceShape::None), Sides(0), Area(0.0), GrossArea(0.0), NetAreaShadowCalc(0.0), Perimeter(0.0),
              Azimuth(0.0), Height(0.0), Reveal(0.0), Tilt(0.0), Width(0.0), HeatTransSurf(false), OutsideHeatSourceTermSchedule(0),
              InsideHeatSourceTermSchedule(0), HeatTransferAlgorithm(HeatTransferModel_NotSet), BaseSurf(0), NumSubSurfaces(0), Zone(0),
              ExtBoundCond(0), LowTempErrCount(0), HighTempErrCount(0), ExtSolar(false), ExtWind(false), IntConvCoeff(0),
              EMSOverrideIntConvCoef(false), EMSValueForIntConvCoef(0.0), ExtConvCoeff(0), EMSOverrideExtConvCoef(false), EMSValueForExtConvCoef(0.0),
              ViewFactorGround(0.0), ViewFactorSky(0.0), ViewFactorGroundIR(0.0), ViewFactorSkyIR(0.0), OSCPtr(0), OSCMPtr(0),
              SchedShadowSurfIndex(0), ShadowSurfSchedVaries(false), ShadowingSurf(false), IsTransparent(false), SchedMinValue(0.0),
              ShadowSurfDiffuseSolRefl(0.0), ShadowSurfDiffuseVisRefl(0.0), ShadowSurfGlazingFrac(0.0), ShadowSurfGlazingConstruct(0),
              ShadowSurfPossibleObstruction(true), ShadowSurfPossibleReflector(false), ShadowSurfRecSurfNum(0), MaterialMovInsulExt(0),
              MaterialMovInsulInt(0), SchedMovInsulExt(0), SchedMovInsulInt(0), MovInsulIntPresent(false), MovInsulIntPresentPrevTS(false),
              Centroid(0.0, 0.0, 0.0), lcsx(0.0, 0.0, 0.0), lcsy(0.0, 0.0, 0.0), lcsz(0.0, 0.0, 0.0), NewellAreaVector(0.0, 0.0, 0.0),
              NewellSurfaceNormalVector(0.0, 0.0, 0.0), OutNormVec(3, 0.0), SinAzim(0.0), CosAzim(0.0), SinTilt(0.0), CosTilt(0.0), IsConvex(true),
              IsDegenerate(false), VerticesProcessed(false), XShift(0.0), YShift(0.0), shapeCat(ShapeCat::Unknown), plane(0.0, 0.0, 0.0, 0.0),
              activeWindowShadingControl(0), HasShadeControl(false), activeShadedConstruction(0), StormWinConstruction(0),
              activeStormWinShadedConstruction(0), FrameDivider(0), Multiplier(1.0), Shelf(0), TAirRef(ZoneMeanAirTemp), OutDryBulbTemp(0.0),
              OutDryBulbTempEMSOverrideOn(false), OutDryBulbTempEMSOverrideValue(0.0), OutWetBulbTemp(0.0), OutWetBulbTempEMSOverrideOn(false),
              OutWetBulbTempEMSOverrideValue(0.0), WindSpeed(0.0), WindSpeedEMSOverrideOn(false), WindSpeedEMSOverrideValue(0.0),
              ViewFactorGroundEMSOverrideOn(false), ViewFactorGroundEMSOverrideValue(0.0),

              WindDir(0.0), WindDirEMSOverrideOn(false), WindDirEMSOverrideValue(0.0),

              SchedExternalShadingFrac(false), ExternalShadingSchInd(0), HasSurroundingSurfProperties(false), SurroundingSurfacesNum(0),
              HasLinkedOutAirNode(false), LinkedOutAirNode(0), PenumbraID(-1),

              UNomWOFilm("-              "), UNomFilm("-              "), ExtEcoRoof(false), ExtCavityPresent(false), ExtCavNum(0), IsPV(false),
              IsICS(false), IsPool(false), ICSPtr(0), MirroredSurf(false), IntConvClassification(0), IntConvHcModelEq(0), IntConvHcUserCurveIndex(0),
              OutConvClassification(0), OutConvHfModelEq(0), OutConvHfUserCurveIndex(0), OutConvHnModelEq(0), OutConvHnUserCurveIndex(0),
              OutConvFaceArea(0.0), OutConvFacePerimeter(0.0), OutConvFaceHeight(0.0), IntConvZoneWallHeight(0.0), IntConvZonePerimLength(0.0),
              IntConvZoneHorizHydrDiam(0.0), IntConvWindowWallRatio(0.0), IntConvWindowLocation(InConvWinLoc_NotSet),
              IntConvSurfGetsRadiantHeat(false), IntConvSurfHasActiveInIt(false), IsRadSurfOrVentSlabOrPool(false), GenericContam(0.0),
              SolarEnclIndex(0), SolarEnclSurfIndex(0), IsAirBoundarySurf(false)
        {
        }

    public: // Methods
            // Set Precomputed Parameters
        void set_computed_geometry();

        void SetOutBulbTempAt(EnergyPlusData &state);

        void SetWindDirAt(Real64 const fac);

        void SetWindSpeedAt(EnergyPlusData &state, Real64 const fac);

        Real64 getInsideAirTemperature(EnergyPlusData &state, const int t_SurfNum) const;

        static Real64 getInsideIR(const int t_SurfNum);

        Real64 getOutsideAirTemperature(EnergyPlusData &state, int t_SurfNum) const;

        Real64 getOutsideIR(EnergyPlusData &state, int t_SurfNum) const;

        static Real64 getSWIncident(EnergyPlusData &state, int t_SurfNum);

        static Real64 getSWBeamIncident(EnergyPlusData &state, int t_SurfNum);

        static Real64 getSWDiffuseIncident(EnergyPlusData &state, int t_SurfNum);

        int getTotLayers(EnergyPlusData &state) const;

        Real64 get_average_height(EnergyPlusData &state) const;

    private: // Methods
             // Computed Shape Category
        ShapeCat computed_shapeCat() const;

        // Computed Plane
        Plane computed_plane() const;

        // Computed axis-projected 2D surface
        Surface2D computed_surface2d() const;
    };

    struct SurfaceWindowCalc // Calculated window-related values
    {
        // Members
        Array1D<Real64> SolidAngAtRefPt;         // Solid angle subtended by window from daylit ref points 1 and 2
        Array1D<Real64> SolidAngAtRefPtWtd;      // Solid angle subtended by window from ref pts weighted by glare pos factor
        Array2D<Real64> IllumFromWinAtRefPt;     // Illuminance from window at ref pts for window with and w/o shade (lux)
        Array2D<Real64> BackLumFromWinAtRefPt;   // Window background luminance from window wrt ref pts (cd/m2) with and w/o shade (cd/m2)
        Array2D<Real64> SourceLumFromWinAtRefPt; // Window luminance at ref pts for window with and w/o shade (cd/m2)
        Array1D<Real64> WinCenter;               // X,Y,Z coordinates of window center point in building coord system
        Array1D<Real64> ThetaFace;               // Face temperatures of window layers (K)

        Array1D<Real64> OutProjSLFracMult; // Multiplier on sunlit fraction due to shadowing of glass by frame
        // and divider outside projections
        Array1D<Real64> InOutProjSLFracMult; // Multiplier on sunlit fraction due to shadowing of glass by frame
        // and divider inside and outside projections
        Array1D<Real64> EffShBlindEmiss; // Effective emissivity of interior blind or shade
        Array1D<Real64> EffGlassEmiss;   // Effective emissivity of glass adjacent to interior blind or shade

        Array1D<Real64> IllumFromWinAtRefPtRep; // Illuminance from window at reference point N [lux]
        Array1D<Real64> LumWinFromRefPtRep;     // Window luminance as viewed from reference point N [cd/m2]
        // for shadowing of ground by building and obstructions [W/m2]
        Array1D<Real64> ZoneAreaMinusThisSurf; // Zone inside surface area minus this surface and its subsurfaces
        // for floor/wall/ceiling (m2)
        Array1D<Real64> ZoneAreaReflProdMinusThisSurf; // Zone product of inside surface area times vis reflectance
        // minus this surface and its subsurfaces,
        // for floor/wall/ceiling (m2)

        BSDFWindowDescript ComplexFen; // Data for complex fenestration, see DataBSDFWindow.cc for declaration

        // Default Constructor
        SurfaceWindowCalc()
            : WinCenter(3, 0.0), ThetaFace(10, 296.15), OutProjSLFracMult(24, 1.0), InOutProjSLFracMult(24, 1.0), EffShBlindEmiss(MaxSlatAngs, 0.0),
              EffGlassEmiss(MaxSlatAngs, 0.0), ZoneAreaMinusThisSurf(3, 0.0), ZoneAreaReflProdMinusThisSurf(3, 0.0)
        {
        }
    };

    struct FrameDividerProperties
    {
        // Members
        std::string Name;          // Name of frame/divider
        Real64 FrameWidth;         // Average width of frame in plane of window {m}
        Real64 FrameProjectionOut; // Distance normal to window between outside face of outer pane
        //  and outside of frame {m}
        Real64 FrameProjectionIn; // Distance normal to window between inside face of inner pane
        //  and inside of frame {m}
        Real64 FrameConductance;          // Effective conductance of frame (no air films) {W/m2-K}
        Real64 FrameEdgeWidth;            // default 2.5 in ! Width of glass edge region near frame {m}
        Real64 FrEdgeToCenterGlCondRatio; // Ratio of frame edge of glass conductance (without air films) to
        // center of glass conductance (without air films)
        Real64 FrameSolAbsorp;       // Solar absorptance of frame corrected for self-shading
        Real64 FrameVisAbsorp;       // Visible absorptance of frame corrected for self-shading
        Real64 FrameEmis;            // Thermal emissivity of frame
        int DividerType;             // Type of divider {DividedLite or Suspended (between-glass}
        Real64 DividerWidth;         // Average width of divider in plane of window {m}
        int HorDividers;             // Number of horizontal dividers
        int VertDividers;            // Number of vertical dividers
        Real64 DividerProjectionOut; // Distance normal to window between outside face of outer pane
        //  and outside of divider {m}
        Real64 DividerProjectionIn; // Distance normal to window between inside face of inner pane
        //  and inside of divider {m}
        Real64 DividerEdgeWidth;           // default 2.5 in ! Width of glass edge region near divider
        Real64 DividerConductance;         // Effective conductance of divider (no air films) {W/m2-K}
        Real64 DivEdgeToCenterGlCondRatio; // Ratio of divider edge of glass conductance (without air films) to
        // center of glass conductance (without air films)
        Real64 DividerSolAbsorp; // Solar absorptance of divider corrected for self-shading
        Real64 DividerVisAbsorp; // Visible absorptance of divider corrected for self-shading
        Real64 DividerEmis;      // Thermal emissivity of divider
        int MullionOrientation;  // Horizontal or Vertical; used only for windows with two glazing systems
        //  divided by a mullion; obtained from Window5 data file.
        Real64 OutsideRevealSolAbs; // Solar absorptance of outside reveal
        Real64 InsideSillDepth;     // Inside sill depth (m)
        Real64 InsideReveal;        // Inside reveal (m)
        Real64 InsideSillSolAbs;    // Solar absorptance of inside sill
        Real64 InsideRevealSolAbs;  // Solar absorptance of inside reveal

        // Default Constructor
        FrameDividerProperties()
            : FrameWidth(0.0), FrameProjectionOut(0.0), FrameProjectionIn(0.0), FrameConductance(0.0), FrameEdgeWidth(0.06355),
              FrEdgeToCenterGlCondRatio(1.0), FrameSolAbsorp(0.0), FrameVisAbsorp(0.0), FrameEmis(0.9), DividerType(0), DividerWidth(0.0),
              HorDividers(0), VertDividers(0), DividerProjectionOut(0.0), DividerProjectionIn(0.0), DividerEdgeWidth(0.06355),
              DividerConductance(0.0), DivEdgeToCenterGlCondRatio(1.0), DividerSolAbsorp(0.0), DividerVisAbsorp(0.0), DividerEmis(0.9),
              MullionOrientation(0), OutsideRevealSolAbs(0.0), InsideSillDepth(0.0), InsideReveal(0.0), InsideSillSolAbs(0.0), InsideRevealSolAbs(0.0)
        {
        }
    };

    struct StormWindowData
    {
        // Members
        int BaseWindowNum;       // Surface number of associated exterior window
        int StormWinMaterialNum; // Material number of storm window glass
        Real64 StormWinDistance; // Distance between storm window glass and adjacent glass (m)
        int DateOn;              // Date (julian) storm window is put on
        int MonthOn;             // Month storm window is put on
        int DayOfMonthOn;        // Day of month storm window is put on
        int DateOff;             // Date (julian) storm window is taken off
        int MonthOff;            // Month storm window is taken off
        int DayOfMonthOff;       // Day of month storm window is taken off

        // Default Constructor
        StormWindowData()
            : BaseWindowNum(0), StormWinMaterialNum(0), StormWinDistance(0.0), DateOn(0), MonthOn(0), DayOfMonthOn(0), DateOff(0), MonthOff(0),
              DayOfMonthOff(0)
        {
        }
    };

    struct WindowShadingControlData
    {
        // Members
        std::string Name;   // User supplied name of this set of shading control data
        int ZoneIndex;      // number of the zone referenced
        int SequenceNumber; // Shading control sequence number
        int ShadingType;    // Shading type (InteriorShade, SwitchableGlazing,
        //  CHARACTER(len=32) :: ShadingType    = ' ' ! Shading type (InteriorShade, SwitchableGlazing,
        //  ExteriorShade,InteriorBlind,ExteriorBlind,BetweenGlassShade,
        //  BetweenGlassBlind, or ExteriorScreen)
        int getInputShadedConstruction; // Pointer to the shaded construction (for ShadingType=ExteriorScreen,InteriorShade,
        //  ExteriorShade,BetweenGlassShade,InteriorBlind,ExteriorBlind,BetweenGlassBlind;
        //  this must be a window construction with a screen, shade or blind layer)
        // this is only used during GetInput and should not be used during timestep calculations
        int ShadingDevice; // Pointer to the material for the shading device (for ShadingType=InteriorShade,
        //  ExteriorShade,BetweenGlassShade,InteriorBlind,ExteriorBlind,BetweenGlassBlind,
        //  ExteriorScreen;
        //  this must be a Material:WindowShade, Material:WindowScreen, or Material:WindowBlind
        int ShadingControlType; // Takes one of the following values that specifies type of shading control
        //  CHARACTER(len=60) :: ShadingControlType =' ' ! Takes one of the following values that specifies type of shading control
        // (control is active only when schedule value = 1; if no schedule
        // specified, schedule value defaults to 1)
        //  AlwaysOn: always shaded; not affected by schedule
        //  AlwaysOff: never shaded; not affected by schedule
        //  OnIfScheduleAllows: unshaded if sch val = 0, shaded if = 1
        //  OnIfHighSolarOnWindow: shaded if incident direct + diffuse > setpoint (W/m2 of window)
        //  OnIfHighHorizontalSolar: shaded if direct + diffuse horizontal solar > setpoint
        //   (W/m2 of ground)
        //  OnIfHighOutsideAirTemp: shaded if outside drybulb > setpoint (C)
        //  OnIfHighZoneAirTemp: shaded if previous time step zone temperature > setpoint (C)
        //  OnIfHighZoneCooling: shaded if previous time step zone cooling rate > setpoint (W)
        //  OnIfHighGlare: shaded if total daylight glare index at first daylighting reference point
        //   from all exterior windows in zone > maximum glare specified in daylighting
        //   input for zone.
        //  MeetDaylightIlluminanceSetpoint: shading is adjusted to just meet illuminance setpoint
        //   at first reference point (only for ShadingType=SwitchableGlazing)
        //       The following three controls are used primarily to reduce zone heating load. They
        //       can be used with any shading type but are most appropriate for opaque interior
        //       or exterior shades with a high insulating value ("opaque movable insulation").
        //  OnNightIfLowOutsideTemp/OffDay: shaded at night if outside temp < setpoint (C)
        //  OnNightIfLowInsideTemp/OffDay: shaded at night if previous time step zone air temp < setpoint (C)
        //  OnNightIfHeating/OffDay: shaded  at night if previous time step zone heating rate > setpoint (W)
        //       The following two controls are used to reduce zone heating and cooling loads.
        //       They can be used with any shading type but are most appropriate for translucent
        //       interior or exterior shades with a high insulating value ("translucent movable insulation")
        //  OnNightIfLowOutsideTemp/OnDayIfCooling: shaded at night if outside temp < setpoint (C);
        //                                         shaded daytime if prev. time step cooling rate > 0
        //  OnNightIfHeating/OnDayIfCooling: shaded at night if prev. time step heating rate > setpoint (W);
        //                                         shaded daytime if prev. time step cooling rate > 0
        //       The following two controls are used to reduce zone cooling load. They can be used
        //       with any shading type but are most appropriate for interior or exterior blinds, interior
        //       or exterior shades with low insulating value, or switchable glazing.
        //  OffNight/OnDayIfCoolingAndHighSolarOnWindow: shading off at night; shading on daytime if
        //                                         solar on window > setpoint (W/m2 of window) and
        //                                         prev. time step cooling rate > 0
        //  OnNight/OnDayIfCoolingAndHighSolarOnWindow: shading on at night; shading on daytime if
        //                                         solar on window > setpoint (W/m2 of window) and
        //                                         prev. time step cooling rate > 0
        int Schedule; // Pointer to schedule of 0 and 1 values: 0 => window is not shaded;
        //  1 => window is shaded if Type=Schedule or Type = ScheduleAnd...
        // and setpoint is exceeded.
        Real64 SetPoint; // Control setpoint (dimension depends on Trigger:
        //  W/m2 of window area for solar on window,
        //  W/m2 of ground area for horizontal solar,
        //  deg C for air temp, W for zone heating and
        //  cooling rate). Not used for Shading Control Type =
        //  MeetDaylightIlluminanceSetpoint or OnIfHighGlare.
        Real64 SetPoint2; // Second control setpoint for control types that take two setpoints.
        //   Dimension is deg C or W/m2.
        bool ShadingControlIsScheduled; // True if shading control has a schedule
        bool GlareControlIsActive;      // True if shading control to reduce daylight glare is active
        int SlatAngleSchedule;          // Pointer to schedule of slat angle values between 0.0 and 180.0 degrees
        int SlatAngleControlForBlinds;  // Takes one of the following values that specifies
                                        //  CHARACTER(len=32) :: SlatAngleControlForBlinds = ' ' ! Takes one of the following values that specifies
                                        //  how slat angle is controled in a blind when ShadingType =
                                        //  InteriorBlind, ExteriorBlind or BetweenGlassBlind.
                                        //  FixedSlatAngle: the slat angle is fixed at the constant value given in the
                                        //    associated Material:WindowBlind
                                        //  ScheduledSlatAngle: the slat angle in degrees between 1 and 180 is given
                                        //    by the schedule with index SlatAngleSchedule
                                        //  BlockBeamSolar: if beam solar is incident on the window, and a blind is on the
                                        //    window, the slat angle is adjusted to just block beam solar; otherwise the
                                        //    slat angle is set to the value given in the associated Material:WindowBlind.
        std::string DaylightingControlName;    // string holding the Daylighting Control Object Name string
        int DaylightControlIndex;              // Pointer to the array of Daylighting Controls
        bool MultiSurfaceCtrlIsGroup;          // True if Group, False if Sequential - type of control order when multiple surfaces are referenced
        int FenestrationCount;                 // count of fenestration references
        Array1D<std::string> FenestrationName; // string holding list of fenestration surfaces
        Array1D_int FenestrationIndex;         // Pointers to fenestration surfaces

        // Default Constructor
        WindowShadingControlData()
            : ZoneIndex(0), SequenceNumber(0), ShadingType(WSC_ST_NoShade), getInputShadedConstruction(0), ShadingDevice(0), ShadingControlType(0),
              Schedule(0), SetPoint(0.0), SetPoint2(0.0), ShadingControlIsScheduled(false), GlareControlIsActive(false), SlatAngleSchedule(0),
              SlatAngleControlForBlinds(0), DaylightControlIndex(0), MultiSurfaceCtrlIsGroup(false), FenestrationCount(0)
        {
        }
    };

    struct OSCData
    {
        // Members
        std::string Name;                  // Name of OSC
        Real64 ConstTemp;                  // User selected constant temperature (degrees C)
        Real64 ConstTempCoef;              // Coefficient modifying the user selected constant temperature
        Real64 ExtDryBulbCoef;             // Coefficient modifying the external dry bulb temperature
        Real64 GroundTempCoef;             // Coefficient modifying the ground temperature
        Real64 SurfFilmCoef;               // Combined convective/radiative film coefficient if >0, else use other coefficients
        Real64 WindSpeedCoef;              // Coefficient modifying the wind speed term (s/m)
        Real64 ZoneAirTempCoef;            // Coefficient modifying the zone air temperature part of the equation
        std::string ConstTempScheduleName; // Schedule name for scheduled outside temp
        int ConstTempScheduleIndex;        // Index for scheduled outside temp.
        bool SinusoidalConstTempCoef;      // If true then ConstTempCoef varies by sine wave
        Real64 SinusoidPeriod;             // period of sine wave variation  (hr)
        Real64 TPreviousCoef;              // Coefficient modifying the OSC temp from the previous timestep (dimensionless)
        Real64 TOutsideSurfPast;           // Ouside surface temperature from previous timestep {C}
        Real64 MinTempLimit;               // Minimum limit on OSC temp {deg C}
        Real64 MaxTempLimit;               // Maximum limit on OSC temp {deg C}
        bool MinLimitPresent;              // If TRUE then apply minimum limit on calculated OSC temp
        bool MaxLimitPresent;              // If TRUE then apply maximum limit on calculated OSC temp
        Real64 OSCTempCalc;                // Result of calculated temperature using OSC (degrees C)

        // Default Constructor
        OSCData()
            : ConstTemp(0.0), ConstTempCoef(0.0), ExtDryBulbCoef(0.0), GroundTempCoef(0.0), SurfFilmCoef(0.0), WindSpeedCoef(0.0),
              ZoneAirTempCoef(0.0), ConstTempScheduleIndex(0), SinusoidalConstTempCoef(false), SinusoidPeriod(0.0), TPreviousCoef(0.0),
              TOutsideSurfPast(0.0), MinTempLimit(0.0), MaxTempLimit(0.0), MinLimitPresent(false), MaxLimitPresent(false), OSCTempCalc(0.0)
        {
        }
    };

    struct OSCMData
    {
        // Members
        std::string Name;             // Name of OSCM
        std::string Class;            // type of Model for OSCM
        Real64 TConv;                 // Temperature of bulk air at other side face (degrees C)
        bool EMSOverrideOnTConv;      // if true then EMS calling for convection bulk air temp override
        Real64 EMSOverrideTConvValue; // value for convection air temp when overridden
        Real64 HConv;                 // Convection coefficient (W/m2-K)
        bool EMSOverrideOnHConv;      // if true then EMS calling for convection coef override
        Real64 EMSOverrideHConvValue; // value to use for convection coef when overridden
        Real64 TRad;                  // Effective temperature of surfaces exposed to other side face (degrees C)
        bool EMSOverrideOnTRad;       // if true then EMS calling for radiation temp override
        Real64 EMSOverrideTRadValue;  // value to use for rad temp when overridden
        Real64 HRad;                  // Linearized Radiation coefficient (W/m2-K)
        bool EMSOverrideOnHrad;       // if true then EMS calling for radiation coef override
        Real64 EMSOverrideHradValue;  // value to use for rad coef when overridden

        // Default Constructor
        OSCMData()
            : TConv(20.0), EMSOverrideOnTConv(false), EMSOverrideTConvValue(0.0), HConv(4.0), EMSOverrideOnHConv(false), EMSOverrideHConvValue(0.0),
              TRad(20.0), EMSOverrideOnTRad(false), EMSOverrideTRadValue(0.0), HRad(4.0), EMSOverrideOnHrad(false), EMSOverrideHradValue(0.0)
        {
        }
    };

    struct ConvectionCoefficient
    {
        // Members
        int WhichSurface;         // Which surface number this is applied to
        std::string SurfaceName;  // Which surface (name)
        int OverrideType;         // Override type, 1=value, 2=schedule, 3=model, 4=user curve
        Real64 OverrideValue;     // User specified value
        std::string ScheduleName; // Which surface (name)
        int ScheduleIndex;        // if type="schedule" is used
        int UserCurveIndex;       // if type=UserCurve is used
        int HcModelEq;            // if type is one of specific model equations

        // Default Constructor
        ConvectionCoefficient() : WhichSurface(0), OverrideType(0), OverrideValue(0.0), ScheduleIndex(0), UserCurveIndex(0), HcModelEq(0)
        {
        }
    };

    struct ShadingVertexData
    {
        // Members
        int NVert;
        Array1D<Real64> XV;
        Array1D<Real64> YV;
        Array1D<Real64> ZV;

        // Default Constructor
        ShadingVertexData()
        {
        }
    };

    struct ExtVentedCavityStruct
    {
        // Members
        // from input data
        std::string Name;
        std::string OSCMName; // OtherSideConditionsModel
        int OSCMPtr;          // OtherSideConditionsModel index
        Real64 Porosity;      // fraction of absorber plate [--]
        Real64 LWEmitt;       // Thermal Emissivity of Baffle Surface [dimensionless]
        Real64 SolAbsorp;     // Solar Absorbtivity of Baffle Surface [dimensionless]
        int BaffleRoughness;  // surface roughness for exterior convection calcs.
        Real64 PlenGapThick;  // Depth of Plenum Behind Baffle [m]
        int NumSurfs;         // a single baffle can have multiple surfaces underneath it
        Array1D_int SurfPtrs; // = 0  ! array of pointers for participating underlying surfaces
        Real64 HdeltaNPL;     // Height scale for Cavity bouyancy  [m]
        Real64 AreaRatio;     // Ratio of actual surface are to projected surface area [dimensionless]
        Real64 Cv;            // volume-based effectiveness of openings for wind-driven vent when Passive
        Real64 Cd;            // discharge coefficient of openings for bouyancy-driven vent when Passive
        // data from elswhere and calculated
        Real64 ActualArea;  // Overall Area of Collect with surface corrugations.
        Real64 ProjArea;    // Overall Area of Collector projected, as if flat [m2]
        Vector Centroid;    // computed centroid
        Real64 TAirCav;     // modeled drybulb temperature for air between baffle and wall [C]
        Real64 Tbaffle;     // modeled surface temperature for baffle[C]
        Real64 TairLast;    // Old Value for modeled drybulb temp of air between baffle and wall [C]
        Real64 TbaffleLast; // Old value for modeled surface temperature for baffle [C]
        Real64 HrPlen;      // Modeled radiation coef for OSCM [W/m2-C]
        Real64 HcPlen;      // Modeled Convection coef for OSCM [W/m2-C]
        Real64 MdotVent;    // air mass flow exchanging with ambient when passive.
        Real64 Tilt;        // Tilt from area weighted average of underlying surfaces
        Real64 Azimuth;     // Azimuth from area weighted average of underlying surfaces
        Real64 QdotSource;  // Source/sink term
        // reporting data
        Real64 Isc;              // total incident solar on baffle [W]
        Real64 PassiveACH;       // air changes per hour when passive [1/hr]
        Real64 PassiveMdotVent;  // Total Nat Vent air change rate  [kg/s]
        Real64 PassiveMdotWind;  // Nat Vent air change rate from Wind-driven [kg/s]
        Real64 PassiveMdotTherm; // Nat. Vent air change rate from bouyancy-driven flow [kg/s]

        // Default Constructor
        ExtVentedCavityStruct()
            : OSCMPtr(0), Porosity(0.0), LWEmitt(0.0), SolAbsorp(0.0), BaffleRoughness(1), PlenGapThick(0.0), NumSurfs(0), HdeltaNPL(0.0),
              AreaRatio(0.0), Cv(0.0), Cd(0.0), ActualArea(0.0), ProjArea(0.0), Centroid(0.0, 0.0, 0.0), TAirCav(0.0), Tbaffle(0.0), TairLast(20.0),
              TbaffleLast(20.0), HrPlen(0.0), HcPlen(0.0), MdotVent(0.0), Tilt(0.0), Azimuth(0.0), QdotSource(0.0), Isc(0.0), PassiveACH(0.0),
              PassiveMdotVent(0.0), PassiveMdotWind(0.0), PassiveMdotTherm(0.0)
        {
        }
    };

    struct SurfaceSolarIncident
    {
        // Members
        std::string Name;
        int SurfPtr;   // surface pointer
        int ConstrPtr; // construction pointer
        int SchedPtr;  // schedule pointer

        // Default Constructor
        SurfaceSolarIncident() : SurfPtr(0), ConstrPtr(0), SchedPtr(0)
        {
        }
    };

    struct FenestrationSolarAbsorbed
    {
        // Members
        std::string Name;
        int SurfPtr;           // surface pointer
        int ConstrPtr;         // construction pointer
        int NumOfSched;        // number of scheduled layers
        Array1D_int SchedPtrs; // pointer to schedules for each layer in construction

        // Default Constructor
        FenestrationSolarAbsorbed() : SurfPtr(0), ConstrPtr(0), NumOfSched(0)
        {
        }
    };

    struct SurfaceLocalEnvironment
    {
        // Members
        std::string Name;
        int SurfPtr;             // surface pointer
        int ExtShadingSchedPtr;  // schedule pointer
        int SurroundingSurfsPtr; // schedule pointer
        int OutdoorAirNodePtr;   // schedule pointer

        // Default Constructor
        SurfaceLocalEnvironment() : SurfPtr(0), ExtShadingSchedPtr(0), SurroundingSurfsPtr(0), OutdoorAirNodePtr(0)
        {
        }
    };

    struct SurroundingSurfProperty
    {
        // Members
        std::string Name;
        Real64 ViewFactor;
        int TempSchNum; // schedule pointer
                        // Default Constructor
        SurroundingSurfProperty() : ViewFactor(0.0), TempSchNum(0)
        {
        }
    };

    struct SurroundingSurfacesProperty
    {
        // Members
        std::string Name;
        Real64 SkyViewFactor;
        int SkyTempSchNum; // schedule pointer
        Real64 GroundViewFactor;
        int GroundTempSchNum;      // schedule pointer
        int TotSurroundingSurface; // Total number of surrounding surfaces defined for an exterior surface
        Array1D<SurroundingSurfProperty> SurroundingSurfs;

        // Default Constructor
        SurroundingSurfacesProperty() : SkyViewFactor(-1.0), SkyTempSchNum(0), GroundViewFactor(-1.0), GroundTempSchNum(0), TotSurroundingSurface(0)
        {
        }
    };

    struct IntMassObject
    {
        // Members
        std::string Name;
        std::string ZoneOrZoneListName; // zone or zone list name
        int ZoneOrZoneListPtr;          // pointer to a zone list
        int NumOfZones;                 // number of zones in a zone list
        int Construction;               // pointer to contruction object
        Real64 GrossArea;               // internal surface area, [m2]
        bool ZoneListActive;            // flag to a list

        // Default Constructor
        IntMassObject() : ZoneOrZoneListPtr(0), NumOfZones(0), Construction(0), GrossArea(0.0), ZoneListActive(false)
        {
        }
    };

    // Object Data
    extern Array1D<SurfaceData> Surface;
    extern Array1D<SurfaceWindowCalc> SurfaceWindow;
    extern Array1D<FrameDividerProperties> FrameDivider;
    extern Array1D<StormWindowData> StormWindow;
    extern Array1D<WindowShadingControlData> WindowShadingControl;
    extern Array1D<OSCData> OSC;
    extern Array1D<OSCMData> OSCM;
    extern Array1D<ConvectionCoefficient> UserIntConvectionCoeffs;
    extern Array1D<ConvectionCoefficient> UserExtConvectionCoeffs;
    extern Array1D<ShadingVertexData> ShadeV;
    extern Array1D<ExtVentedCavityStruct> ExtVentedCavity;
    extern Array1D<SurfaceSolarIncident> SurfIncSolSSG;
    extern Array1D<FenestrationSolarAbsorbed> FenLayAbsSSG;
    extern Array1D<SurfaceLocalEnvironment> SurfLocalEnvironment;
    extern Array1D<SurroundingSurfacesProperty> SurroundingSurfsProperty;
    extern Array1D<IntMassObject> IntMassObjects;

    // Functions

    // Clears the global data in DataSurfaces.
    // Needed for unit tests, should not be normally called.
    void clear_state();

    void SetSurfaceOutBulbTempAt(EnergyPlusData &state);

    void CheckSurfaceOutBulbTempAt(EnergyPlusData &state);

    void SetSurfaceWindSpeedAt(EnergyPlusData &state);

    void SetSurfaceWindDirAt(EnergyPlusData &state);

    Real64 AbsFrontSide(int SurfNum);

    Real64 AbsBackSide(int SurfNum);

    std::string cSurfaceClass(SurfaceClass ClassNo);

} // namespace DataSurfaces

struct SurfacesData : BaseGlobalStruct
{
    int TotSurfaces = 0;           // Total number of surfaces (walls, floors, roofs, windows, shading surfaces, etc.--everything)
    int TotWindows = 0;            // Total number of windows
    int TotComplexWin = 0;         // Total number of windows with complex optical properties
    int TotStormWin = 0;           // Total number of storm window blocks
    int TotWinShadingControl = 0;  // Total number of window shading control blocks
    int TotIntConvCoeff = 0;       // Total number of interior convection coefficient (overrides)
    int TotExtConvCoeff = 0;       // Total number of exterior convection coefficient (overrides)
    int TotOSC = 0;                // Total number of Other Side Coefficient Blocks
    int TotOSCM = 0;               // Total number of Other Side Conditions Model Blocks.
    int TotExtVentCav = 0;         // Total number of ExteriorNaturalVentedCavity
    int TotSurfIncSolSSG = 0;      // Total number of scheduled surface gains for incident solar radiation on surface
    int TotFenLayAbsSSG = 0;       // Total number of scheduled surface gains for absorbed solar radiation in window layers
    int TotSurfLocalEnv = 0;       // Total number of surface level outdoor air node.
    int Corner = 0;                // Which corner is specified as the first vertex
    int MaxVerticesPerSurface = 4; // Maximum number of vertices allowed for a single surface (default -- can go higher)
    int BuildingShadingCount = 0;  // Total number of Building External Shades
    int FixedShadingCount = 0;     // Total number of Fixed External Shades
    int AttachedShadingCount = 0;  // Total number of Shades attached to Zones
    int ShadingSurfaceFirst = -1;  // Start index of shading surfaces (Building External Shades, Fixed External Shades and Shades attached to Zone)
    int ShadingSurfaceLast = -1;   // End index of shading surfaces (Building External Shades, Fixed External Shades and Shades attached to Zone)
    bool AspectTransform = false;  // Set to true when GeometryTransform object is used
    bool CalcSolRefl = false;      // Set to true when Solar Reflection Calculations object is used
    bool CCW = false;              // True if vertices will be entered in CounterClockWise Order
    bool WorldCoordSystem = false; // True if vertices will be "World Coordinates". False means relative coordinates
    bool DaylRefWorldCoordSystem = false; // True if Daylight Reference Point vertices will be "World Coordinates". False means relative coordinates
    int MaxRecPts = 0;                    // Max number of receiving points on a surface for solar reflection calc
    int MaxReflRays = 0;                  // Max number of rays from a receiving surface for solar reflection calc
    Real64 GroundLevelZ = 0.0;            // Z value of ground level for solar refl calc (m)
    bool AirflowWindows = false;          // TRUE if one or more airflow windows
    bool ShadingTransmittanceVaries = false;   // overall, shading transmittance varies for the building
    Array1D_int InsideGlassCondensationFlag;   // 1 if innermost glass inside surface temp < zone air dew point;  0 otherwise
    Array1D_int InsideFrameCondensationFlag;   // 1 if frame inside surface temp < zone air dew point; 0 otherwise
    Array1D_int InsideDividerCondensationFlag; // 1 if divider inside surface temp < zone air dew point;  0 otherwise
    Array1D_int AdjacentZoneToSurface;         // Array of adjacent zones to each surface
    Array1D<Real64> X0;                        // X-component of translation vector
    Array1D<Real64> Y0;                        // Y-component of translation vector
    Array1D<Real64> Z0;                        // Z-component of translation vector
    Array1D<Real64> EnclSolDB;                 // Factor for diffuse radiation in a zone from beam reflecting from inside surfaces
    Array1D<Real64>
        EnclSolDBSSG; // Factor for diffuse radiation in a zone from beam reflecting from inside surfaces. Used only for scheduled surface gains
    Array1D<Real64> SurfOpaqAI;             // Time step value of factor for beam absorbed on inside of opaque surface
    Array1D<Real64> SurfOpaqAO;             // Time step value of factor for beam absorbed on outside of opaque surface
    Array1D<Real64> SurfBmToBmReflFacObs;   // Factor for incident solar from specular beam refl from obstructions (W/m2)/(W/m2)
    Array1D<Real64> SurfBmToDiffReflFacObs; // Factor for incident solar from diffuse beam refl from obstructions (W/m2)/(W/m2)
    Array1D<Real64> SurfBmToDiffReflFacGnd; // Factor for incident solar from diffuse beam refl from ground
    Array1D<Real64> SurfSkyDiffReflFacGnd;  // sky diffuse reflection view factors from ground
    Array2D<Real64> SurfWinA;               // Time step value of factor for beam absorbed in window glass layers

    // Time step value of factor for diffuse absorbed in window layers
    Array2D<Real64> SurfWinADiffFront;
    Array2D<Real64> SurfWinADiffBack;

    Array2D<Real64> SurfWinACFOverlap; // Time step value of factor for beam
    // absorbed in window glass layers which comes from other windows
    // It happens sometimes that beam enters one window and hits back of
    // second window. It is used in complex fenestration only

    Array1D<Real64> AirSkyRadSplit; // Fractional split between the air and
    // the sky for radiation from the surface
    // Fraction of sky IR coming from sky itself; 1-AirSkyRadSplit comes from the atmosphere.

    Array2D<Real64> SUNCOSHR = Array2D<Real64>(
        24, 3, 0.0); // Hourly values of SUNCOS (solar direction cosines), Autodesk: Init Zero-initialization added to avoid use uninitialized
    Array2D<Real64> ReflFacBmToDiffSolObs;
    Array2D<Real64> ReflFacBmToDiffSolGnd;
    Array2D<Real64> ReflFacBmToBmSolObs;
    Array1D<Real64> ReflFacSkySolObs;
    Array1D<Real64> ReflFacSkySolGnd;
    Array2D<Real64> CosIncAveBmToBmSolObs;
    Array1D<Real64>
        EnclSolDBIntWin; // Value of factor for beam solar entering a zone through interior windows (considered to contribute to diffuse in zone)
    Array1D<Real64> SurfSunlitArea;  // Sunlit area by surface number
    Array1D<Real64> SurfSunlitFrac;  // Sunlit fraction by surface number
    Array1D<Real64> SurfSkySolarInc; // Incident diffuse solar from sky; if CalcSolRefl is true, includes reflection of sky diffuse and beam solar
                                     // from exterior obstructions [W/m2]
    Array1D<Real64> SurfGndSolarInc; // Incident diffuse solar from ground; if CalcSolRefl is true, accounts for shadowing of ground by building and
                                     // obstructions [W/m2]

    std::vector<int> AllHTSurfaceList;          // List of all heat transfer surfaces
    std::vector<int> AllIZSurfaceList;          // List of all interzone heat transfer surfaces
    std::vector<int> AllHTNonWindowSurfaceList; // List of all non-window heat transfer surfaces
    std::vector<int> AllHTWindowSurfaceList;    // List of all window surfaces
    std::vector<int> AllSurfaceListReportOrder; // List of all surfaces - output reporting order

    // Surface Window Heat Balance
    Array1D<Real64> SurfWinTransSolar; // Exterior beam plus diffuse solar transmitted through window, or window plus shade/blind, into zone (W)
    Array1D<Real64> SurfWinBmSolar;    // Exterior beam solar transmitted through window, or window plus blind, into zone (W)
    Array1D<Real64> SurfWinBmBmSolar;  // Exterior beam-to-beam solar transmitted through window, or window plus blind, into zone (W)
    Array1D<Real64> SurfWinBmDifSolar; // Exterior beam-to-diffuse solar transmitted through window, or window plus blind, into zone (W)
    Array1D<Real64> SurfWinDifSolar;   // Exterior diffuse solar transmitted through window, or window plus shade/blind, into zone (W)
    Array1D<Real64> SurfWinHeatGain;   // Total heat gain from window = WinTransSolar + (IR and convection from glazing, or,
    // if interior shade, IR and convection from zone-side of shade plus gap air convection to zone) +
    // (IR convection from frame) + (IR and convection from divider if no interior shade) (W)
    Array1D<Real64> SurfWinHeatTransfer;              // Total heat transfer through the window = WinTransSolar + conduction through glazing and frame
    Array1D<Real64> SurfWinHeatGainRep;               // Equals WinHeatGain when WinHeatGain >= 0.0
    Array1D<Real64> SurfWinHeatLossRep;               // Equals -WinHeatGain when WinHeatGain < 0.0
    Array1D<Real64> SurfWinGainConvGlazToZoneRep;     // component of WinHeatGain convect to zone from glazing (W)
    Array1D<Real64> SurfWinGainIRGlazToZoneRep;       // component of WinHeatGain net IR to zone from glazing (W)
    Array1D<Real64> SurfWinLossSWZoneToOutWinRep;     // component of WinHeatGain shortwave transmit back out (W)
    Array1D<Real64> SurfWinGainFrameDividerToZoneRep; // component of WinHeatGain to zone from frame/divider (W)
    Array1D<Real64> SurfWinGainConvGlazShadGapToZoneRep; // component of WinHeatGain convection to zone from the gap between the inner most glazing
                                                         // and the shade   (W)
    Array1D<Real64> SurfWinGainConvShadeToZoneRep;       // component of WinHeatGain convect to zone from front shade (W)
    Array1D<Real64> SurfWinGainIRShadeToZoneRep;         // component of WinHeatGain net IR to zone from front shade (W)
    Array1D<Real64>
        SurfWinOtherConvGainInsideFaceToZoneRep; // net imbalance of convection heat gain from equivalent Layer window inside face to zone air
    Array1D<Real64> SurfWinGapConvHtFlowRep;     // Convective heat flow from gap in airflow window (W)
    Array1D<Real64> SurfWinShadingAbsorbedSolar; // Exterior beam plus diffuse solar absorbed by window shading device (W)
    Array1D<Real64> SurfWinSysSolTransmittance;  // Effective solar transmittance of window + shading device, if present
    Array1D<Real64> SurfWinSysSolReflectance;    // Effective solar reflectance of window + shading device, if present
    Array1D<Real64> SurfWinSysSolAbsorptance;    // Effective solar absorptance of window + shading device, if present

    void clear_state() override
    {
        this->TotSurfaces = 0;
        this->TotWindows = 0;
        this->TotComplexWin = 0;
        this->TotStormWin = 0;
        this->TotWinShadingControl = 0;
        this->TotIntConvCoeff = 0;
        this->TotExtConvCoeff = 0;
        this->TotOSC = 0;
        this->TotOSCM = 0;
        this->TotExtVentCav = 0;
        this->TotSurfIncSolSSG = 0;
        this->TotFenLayAbsSSG = 0;
        this->TotSurfLocalEnv = 0;
        this->Corner = 0;
        this->MaxVerticesPerSurface = 4;
        this->BuildingShadingCount = 0;
        this->FixedShadingCount = 0;
        this->AttachedShadingCount = 0;
        this->ShadingSurfaceFirst = -1;
        this->ShadingSurfaceLast = -1;
        this->AspectTransform = false;
        this->CalcSolRefl = false;
        this->CCW = false;
        this->WorldCoordSystem = false;
        this->DaylRefWorldCoordSystem = false;
        this->MaxRecPts = 0;
        this->MaxReflRays = 0;
        this->GroundLevelZ = 0.0;
        this->AirflowWindows = false;
        this->ShadingTransmittanceVaries = false;
        this->InsideGlassCondensationFlag.deallocate();
        this->InsideFrameCondensationFlag.deallocate();
        this->InsideDividerCondensationFlag.deallocate();
        this->AdjacentZoneToSurface.deallocate();
        this->X0.deallocate();
        this->Y0.deallocate();
        this->Z0.deallocate();
        this->EnclSolDB.deallocate();
        this->EnclSolDBSSG.deallocate();
        this->SurfOpaqAI.deallocate();
        this->SurfOpaqAO.deallocate();
        this->SurfBmToBmReflFacObs.deallocate();
        this->SurfBmToDiffReflFacObs.deallocate();
        this->SurfBmToDiffReflFacGnd.deallocate();
        this->SurfSkyDiffReflFacGnd.deallocate();
        this->SurfWinA.deallocate();
        this->SurfWinADiffFront.deallocate();
        this->SurfWinADiffBack.deallocate();
        this->SurfWinACFOverlap.deallocate();
        this->AirSkyRadSplit.deallocate();
        this->SUNCOSHR = Array2D<Real64>(24, 3, 0.0);
        this->ReflFacBmToDiffSolObs.deallocate();
        this->ReflFacBmToDiffSolGnd.deallocate();
        this->ReflFacBmToBmSolObs.deallocate();
        this->ReflFacSkySolObs.deallocate();
        this->ReflFacSkySolGnd.deallocate();
        this->CosIncAveBmToBmSolObs.deallocate();
        this->EnclSolDBIntWin.deallocate();
        this->SurfSunlitArea.deallocate();
        this->SurfSunlitFrac.deallocate();
        this->SurfSkySolarInc.deallocate();
        this->SurfGndSolarInc.deallocate();
        this->AllHTSurfaceList.clear();
        this->AllIZSurfaceList.clear();
        this->AllHTNonWindowSurfaceList.clear();
        this->AllHTWindowSurfaceList.clear();
        this->AllSurfaceListReportOrder.clear();
        this->SurfWinTransSolar.deallocate();
        this->SurfWinBmSolar.deallocate();
        this->SurfWinBmBmSolar.deallocate();
        this->SurfWinBmDifSolar.deallocate();
        this->SurfWinDifSolar.deallocate();
        this->SurfWinHeatGain.deallocate();
        this->SurfWinHeatTransfer.deallocate();
        this->SurfWinHeatGainRep.deallocate();
        this->SurfWinHeatLossRep.deallocate();
        this->SurfWinGainConvGlazToZoneRep.deallocate();
        this->SurfWinGainIRGlazToZoneRep.deallocate();
        this->SurfWinLossSWZoneToOutWinRep.deallocate();
        this->SurfWinGainFrameDividerToZoneRep.deallocate();
        this->SurfWinGainConvGlazShadGapToZoneRep.deallocate();
        this->SurfWinGainConvShadeToZoneRep.deallocate();
        this->SurfWinGainIRShadeToZoneRep.deallocate();
        this->SurfWinOtherConvGainInsideFaceToZoneRep.deallocate();
        this->SurfWinGapConvHtFlowRep.deallocate();
        this->SurfWinShadingAbsorbedSolar.deallocate();
        this->SurfWinSysSolTransmittance.deallocate();
        this->SurfWinSysSolReflectance.deallocate();
        this->SurfWinSysSolAbsorptance.deallocate();
    }
};

} // namespace EnergyPlus

#endif
