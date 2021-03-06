// EnergyPlus, Copyright (c) 1996-2020, The Board of Trustees of the University of Illinois,
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

#include <EnergyPlus/Autosizing/CoolingCapacitySizing.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/HVACFan.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/SimAirServingZones.hh>
#include <EnergyPlus/WeatherManager.hh>

namespace EnergyPlus {

Real64 CoolingCapacitySizer::size(EnergyPlusData &state, Real64 _originalValue, bool &errorsFound)
{
    if (!this->checkInitialized(errorsFound)) {
        return 0.0;
    }
    this->preSize(_originalValue);
    std::string DDNameFanPeak = "";
    std::string dateTimeFanPeak = "";
    Real64 DesVolFlow = 0.0;
    Real64 CoilInTemp = -999.0;
    Real64 CoilInHumRat = -999.0;
    Real64 CoilOutTemp = -999.0;
    Real64 CoilOutHumRat = -999.0;
    Real64 FanCoolLoad = 0.0;
    Real64 TotCapTempModFac = 1.0;
    Real64 DXFlowPerCapMinRatio = 1.0;
    Real64 DXFlowPerCapMaxRatio = 1.0;

    if (this->dataEMSOverrideON) {
        this->autoSizedValue = this->dataEMSOverride;
    } else if (this->dataConstantUsedForSizing >= 0 && this->dataFractionUsedForSizing > 0) {
        // back and forth if dataConstantUsedForSizing should be > or >= 0 to make this work for AutoCalculate
        this->autoSizedValue = this->dataConstantUsedForSizing * this->dataFractionUsedForSizing;
    } else {
        if (this->curZoneEqNum > 0) {
            if (!this->wasAutoSized && !this->sizingDesRunThisZone) {
                this->autoSizedValue = _originalValue;
            } else if (this->zoneEqSizing(this->curZoneEqNum).DesignSizeFromParent) {
                this->autoSizedValue = this->zoneEqSizing(this->curZoneEqNum).DesCoolingLoad;
            } else {
                if (this->zoneEqSizing(this->curZoneEqNum).CoolingCapacity) { // Parent object calculated capacity
                    this->autoSizedValue = this->zoneEqSizing(this->curZoneEqNum).DesCoolingLoad;
                    DesVolFlow = this->dataFlowUsedForSizing;
                    CoilInTemp = DataSizing::DataCoilSizingAirInTemp;
                    CoilInHumRat = DataSizing::DataCoilSizingAirInHumRat;
                    CoilOutTemp = DataSizing::DataCoilSizingAirOutTemp;
                    CoilOutHumRat = DataSizing::DataCoilSizingAirOutHumRat;
                    FanCoolLoad = DataSizing::DataCoilSizingFanCoolLoad;
                    TotCapTempModFac = DataSizing::DataCoilSizingCapFT;
                } else {
                    if (UtilityRoutines::SameString(this->compType, "COIL:COOLING:WATER") ||
                        UtilityRoutines::SameString(this->compType, "COIL:COOLING:WATER:DETAILEDGEOMETRY") ||
                        UtilityRoutines::SameString(this->compType, "ZONEHVAC:IDEALLOADSAIRSYSTEM")) {
                        if (this->termUnitIU && (this->curTermUnitSizingNum > 0)) {
                            this->autoSizedValue = this->termUnitSizing(this->curTermUnitSizingNum).DesCoolingLoad;
                        } else if (this->zoneEqFanCoil) {
                            this->autoSizedValue = this->zoneEqSizing(this->curZoneEqNum).DesCoolingLoad;
                        } else {
                            CoilInTemp = this->finalZoneSizing(this->curZoneEqNum).DesCoolCoilInTemp;
                            CoilInHumRat = this->finalZoneSizing(this->curZoneEqNum).DesCoolCoilInHumRat;
                            CoilOutTemp = min(CoilInTemp, this->finalZoneSizing(this->curZoneEqNum).CoolDesTemp);
                            CoilOutHumRat = min(CoilInHumRat, this->finalZoneSizing(this->curZoneEqNum).CoolDesHumRat);
                            this->autoSizedValue =
                                this->finalZoneSizing(this->curZoneEqNum).DesCoolMassFlow *
                                (Psychrometrics::PsyHFnTdbW(CoilInTemp, CoilInHumRat) - Psychrometrics::PsyHFnTdbW(CoilOutTemp, CoilOutHumRat));
                            DesVolFlow = this->finalZoneSizing(this->curZoneEqNum).DesCoolMassFlow / DataEnvironment::StdRhoAir;
                            // add fan heat to coil load
                            FanCoolLoad += this->calcFanDesHeatGain(DesVolFlow);
                            this->autoSizedValue += FanCoolLoad;
                        }
                    } else {
                        DesVolFlow = this->dataFlowUsedForSizing;
                        if (DesVolFlow >= DataHVACGlobals::SmallAirVolFlow) {
                            // each of these IFs now seem the same and can be condensed to just CoilInTemp = set() and CoilInHumRat = set()
                            if (DataSizing::ZoneEqDXCoil) {
                                // ATMixer has priority over Equipment OA vol flow
                                if (this->zoneEqSizing(this->curZoneEqNum).ATMixerVolFlow > 0.0) { // NEW ATMixer coil sizing method
                                    Real64 DesMassFlow = DesVolFlow * DataEnvironment::StdRhoAir;
                                    CoilInTemp =
                                        setCoolCoilInletTempForZoneEqSizing(setOAFracForZoneEqSizing(DesMassFlow, zoneEqSizing(this->curZoneEqNum)),
                                                                            zoneEqSizing(this->curZoneEqNum),
                                                                            finalZoneSizing(this->curZoneEqNum));
                                    CoilInHumRat =
                                        setCoolCoilInletHumRatForZoneEqSizing(setOAFracForZoneEqSizing(DesMassFlow, zoneEqSizing(this->curZoneEqNum)),
                                                                              zoneEqSizing(this->curZoneEqNum),
                                                                              finalZoneSizing(this->curZoneEqNum));
                                } else if (this->zoneEqSizing(this->curZoneEqNum).OAVolFlow > 0.0) {
                                    CoilInTemp = this->finalZoneSizing(this->curZoneEqNum).DesCoolCoilInTemp;
                                    CoilInHumRat = this->finalZoneSizing(this->curZoneEqNum).DesCoolCoilInHumRat;
                                } else {
                                    CoilInTemp = this->finalZoneSizing(this->curZoneEqNum)
                                                     .ZoneRetTempAtCoolPeak; // Question whether zone equipment should use return temp for sizing
                                    CoilInHumRat = this->finalZoneSizing(this->curZoneEqNum).ZoneHumRatAtCoolPeak;
                                }
                            } else if (this->zoneEqFanCoil) {
                                // use fan coil flow (i.e., set by parent) or flow used during sizing?
                                Real64 DesMassFlow = this->finalZoneSizing(this->curZoneEqNum).DesCoolMassFlow;
                                CoilInTemp =
                                    setCoolCoilInletTempForZoneEqSizing(setOAFracForZoneEqSizing(DesMassFlow, zoneEqSizing(this->curZoneEqNum)),
                                                                        zoneEqSizing(this->curZoneEqNum),
                                                                        finalZoneSizing(this->curZoneEqNum));
                                CoilInHumRat =
                                    setCoolCoilInletHumRatForZoneEqSizing(setOAFracForZoneEqSizing(DesMassFlow, zoneEqSizing(this->curZoneEqNum)),
                                                                          zoneEqSizing(this->curZoneEqNum),
                                                                          finalZoneSizing(this->curZoneEqNum));
                            } else {
                                CoilInTemp = this->finalZoneSizing(this->curZoneEqNum).DesCoolCoilInTemp;
                                CoilInHumRat = this->finalZoneSizing(this->curZoneEqNum).DesCoolCoilInHumRat;
                            }
                            CoilOutTemp = min(CoilInTemp, this->finalZoneSizing(this->curZoneEqNum).CoolDesTemp);
                            CoilOutHumRat = min(CoilInHumRat, this->finalZoneSizing(this->curZoneEqNum).CoolDesHumRat);
                            int TimeStepNumAtMax = this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtCoolMax;
                            int DDNum = this->finalZoneSizing(this->curZoneEqNum).CoolDDNum;
                            Real64 OutTemp = 0.0;
                            if (DDNum > 0 && TimeStepNumAtMax > 0) {
                                OutTemp = DataSizing::DesDayWeath(DDNum).Temp(TimeStepNumAtMax);
                            }
                            Real64 rhoair =
                                Psychrometrics::PsyRhoAirFnPbTdbW(DataEnvironment::StdBaroPress, CoilInTemp, CoilInHumRat, this->callingRoutine);
                            Real64 CoilInEnth = Psychrometrics::PsyHFnTdbW(CoilInTemp, CoilInHumRat);
                            Real64 CoilOutEnth = Psychrometrics::PsyHFnTdbW(CoilOutTemp, CoilOutHumRat);
                            Real64 PeakCoilLoad = max(0.0, (rhoair * DesVolFlow * (CoilInEnth - CoilOutEnth)));
                            if (this->zoneEqFanCoil) {
                                PeakCoilLoad = max(0.0, (DataEnvironment::StdRhoAir * DesVolFlow * (CoilInEnth - CoilOutEnth)));
                            } else if (this->zoneEqUnitVent) {
                                PeakCoilLoad = max(0.0, (DataEnvironment::StdRhoAir * DesVolFlow * (CoilInEnth - CoilOutEnth)));
                            }
                            // add fan heat to coil load
                            FanCoolLoad += this->calcFanDesHeatGain(DesVolFlow);
                            PeakCoilLoad += FanCoolLoad;
                            Real64 CpAir = Psychrometrics::PsyCpAirFnW(CoilInHumRat);
                            // adjust coil inlet/outlet temp with fan temperature rise
                            if (this->dataDesAccountForFanHeat) {
                                if (DataSizing::DataFanPlacement == DataSizing::zoneFanPlacement::zoneBlowThru) {
                                    CoilInTemp += FanCoolLoad / (CpAir * DataEnvironment::StdRhoAir * DesVolFlow);
                                } else if (DataSizing::DataFanPlacement == DataSizing::zoneFanPlacement::zoneDrawThru) {
                                    CoilOutTemp -= FanCoolLoad / (CpAir * DataEnvironment::StdRhoAir * DesVolFlow);
                                }
                            }
                            Real64 CoilInWetBulb =
                                Psychrometrics::PsyTwbFnTdbWPb(CoilInTemp, CoilInHumRat, DataEnvironment::StdBaroPress, this->callingRoutine);
                            if (this->dataTotCapCurveIndex > 0) {
                                TotCapTempModFac = CurveManager::CurveValue(state, this->dataTotCapCurveIndex, CoilInWetBulb, OutTemp);
                            } else if (this->dataTotCapCurveValue > 0) {
                                TotCapTempModFac = this->dataTotCapCurveValue;
                            } else {
                                TotCapTempModFac = 1.0;
                            }
                            if (TotCapTempModFac > 0.0) {
                                this->autoSizedValue = PeakCoilLoad / TotCapTempModFac;
                            } else {
                                this->autoSizedValue = PeakCoilLoad;
                            }
                            // save these conditions to use when this->zoneEqSizing(this->curZoneEqNum).CoolingCapacity = true
                            DataSizing::DataCoilSizingAirInTemp = CoilInTemp;
                            DataSizing::DataCoilSizingAirInHumRat = CoilInHumRat;
                            DataSizing::DataCoilSizingAirOutTemp = CoilOutTemp;
                            DataSizing::DataCoilSizingAirOutHumRat = CoilOutHumRat;
                            DataSizing::DataCoilSizingFanCoolLoad = FanCoolLoad;
                            DataSizing::DataCoilSizingCapFT = TotCapTempModFac;
                        } else {
                            this->autoSizedValue = 0.0;
                            CoilOutTemp = -999.0;
                        }
                    }
                }
                this->autoSizedValue = this->autoSizedValue * this->dataFracOfAutosizedCoolingCapacity;
                this->dataDesAccountForFanHeat = true; // reset for next water coil
                if (DataGlobals::DisplayExtraWarnings && this->autoSizedValue <= 0.0) {
                    ShowWarningMessage(this->callingRoutine + ": Potential issue with equipment sizing for " + this->compType + ' ' + this->compName);
                    ShowContinueError("...Rated Total Cooling Capacity = " + General::TrimSigDigits(this->autoSizedValue, 2) + " [W]");
                    if (this->zoneEqSizing(this->curZoneEqNum).CoolingCapacity) {
                        ShowContinueError("...Capacity passed by parent object to size child component = " +
                                          General::TrimSigDigits(this->autoSizedValue, 2) + " [W]");
                    } else {
                        if (UtilityRoutines::SameString(this->compType, "COIL:COOLING:WATER") ||
                            UtilityRoutines::SameString(this->compType, "COIL:COOLING:WATER:DETAILEDGEOMETRY") ||
                            UtilityRoutines::SameString(this->compType, "ZONEHVAC:IDEALLOADSAIRSYSTEM")) {
                            if (this->termUnitIU || this->zoneEqFanCoil) {
                                ShowContinueError("...Capacity passed by parent object to size child component = " +
                                                  General::TrimSigDigits(this->autoSizedValue, 2) + " [W]");
                            } else {
                                ShowContinueError("...Air flow rate used for sizing = " + General::TrimSigDigits(DesVolFlow, 5) + " [m3/s]");
                                ShowContinueError("...Coil inlet air temperature used for sizing = " + General::TrimSigDigits(CoilInTemp, 2) +
                                                  " [C]");
                                ShowContinueError("...Coil outlet air temperature used for sizing = " + General::TrimSigDigits(CoilOutTemp, 2) +
                                                  " [C]");
                            }
                        } else {
                            if (CoilOutTemp > -999.0) {
                                ShowContinueError("...Air flow rate used for sizing = " + General::TrimSigDigits(DesVolFlow, 5) + " [m3/s]");
                                ShowContinueError("...Coil inlet air temperature used for sizing = " + General::TrimSigDigits(CoilInTemp, 2) +
                                                  " [C]");
                                ShowContinueError("...Coil outlet air temperature used for sizing = " + General::TrimSigDigits(CoilOutTemp, 2) +
                                                  " [C]");
                            } else {
                                ShowContinueError("...Capacity used to size child component set to 0 [W]");
                            }
                        }
                    }
                }
            }
        } else if (this->curSysNum > 0) {
            if (!this->wasAutoSized && !this->sizingDesRunThisAirSys) {
                this->autoSizedValue = _originalValue;
            } else {
                Real64 OutAirFrac = 0.0;
                this->dataFracOfAutosizedCoolingCapacity = 1.0;
                if (this->oaSysFlag) {
                    this->autoSizedValue = this->oaSysEqSizing(this->curOASysNum).DesCoolingLoad;
                    DesVolFlow = this->dataFlowUsedForSizing;
                } else if (this->airLoopSysFlag) {
                    this->autoSizedValue = this->unitarySysEqSizing(this->curSysNum).DesCoolingLoad;
                    DesVolFlow = this->dataFlowUsedForSizing;
                    CoilInTemp = DataSizing::DataCoilSizingAirInTemp;
                    CoilInHumRat = DataSizing::DataCoilSizingAirInHumRat;
                    CoilOutTemp = DataSizing::DataCoilSizingAirOutTemp;
                    CoilOutHumRat = DataSizing::DataCoilSizingAirOutHumRat;
                    FanCoolLoad = DataSizing::DataCoilSizingFanCoolLoad;
                    TotCapTempModFac = DataSizing::DataCoilSizingCapFT;
                    if (coilSelectionReportObj->isCompTypeCoil(this->compType)) {
                        coilSelectionReportObj->setCoilEntAirHumRat(this->compName, this->compType, CoilInHumRat);
                        coilSelectionReportObj->setCoilEntAirTemp(state, this->compName, this->compType, CoilInTemp, this->curSysNum, this->curZoneEqNum);
                        coilSelectionReportObj->setCoilLvgAirTemp(this->compName, this->compType, CoilOutTemp);
                        coilSelectionReportObj->setCoilLvgAirHumRat(this->compName, this->compType, CoilOutHumRat);
                    }
                } else if (this->curOASysNum > 0 && this->outsideAirSys(this->curOASysNum).AirLoopDOASNum > -1) {
                    DesVolFlow =
                        this->airloopDOAS[outsideAirSys(this->curOASysNum).AirLoopDOASNum].SizingMassFlow / DataEnvironment::StdRhoAir;
                    if (this->airloopDOAS[outsideAirSys(this->curOASysNum).AirLoopDOASNum].DXCoilFlag) {
                        this->autoSizedValue = DesVolFlow / 0.00005;
                    } else {
                        CoilInTemp = this->airloopDOAS[outsideAirSys(this->curOASysNum).AirLoopDOASNum].SizingCoolOATemp;
                        if (this->airloopDOAS[this->outsideAirSys(this->curOASysNum).AirLoopDOASNum].m_FanIndex > -1 &&
                            this->airloopDOAS[this->outsideAirSys(this->curOASysNum).AirLoopDOASNum].FanBlowTroughFlag &&
                            this->airloopDOAS[this->outsideAirSys(this->curOASysNum).AirLoopDOASNum].m_FanTypeNum ==
                                SimAirServingZones::Fan_System_Object) {
                            int FanIndex = this->airloopDOAS[this->outsideAirSys(this->curOASysNum).AirLoopDOASNum].m_FanIndex;
                            Real64 DeltaT = HVACFan::fanObjs[FanIndex]->getFanDesignTemperatureRise();
                            CoilInTemp += DeltaT;
                        }
                        CoilInHumRat = this->airloopDOAS[outsideAirSys(this->curOASysNum).AirLoopDOASNum].SizingCoolOAHumRat;
                        CoilOutTemp = this->airloopDOAS[outsideAirSys(this->curOASysNum).AirLoopDOASNum].PrecoolTemp;
                        CoilOutHumRat = this->airloopDOAS[outsideAirSys(this->curOASysNum).AirLoopDOASNum].PrecoolHumRat;
                        this->autoSizedValue =
                            DesVolFlow * DataEnvironment::StdRhoAir *
                            (Psychrometrics::PsyHFnTdbW(CoilInTemp, CoilInHumRat) - Psychrometrics::PsyHFnTdbW(CoilOutTemp, CoilOutHumRat));
                    }
                } else {
                    CheckSysSizing(this->compType, this->compName);
                    DesVolFlow = this->dataFlowUsedForSizing;
                    Real64 NominalCapacityDes = 0.0;
                    if (this->finalSysSizing(this->curSysNum).CoolingCapMethod == DataSizing::FractionOfAutosizedCoolingCapacity) {
                        this->dataFracOfAutosizedCoolingCapacity = this->finalSysSizing(this->curSysNum).FractionOfAutosizedCoolingCapacity;
                    }
                    if (this->finalSysSizing(this->curSysNum).CoolingCapMethod == DataSizing::CapacityPerFloorArea) {
                        NominalCapacityDes = this->finalSysSizing(this->curSysNum).CoolingTotalCapacity;
                        this->autoSizedValue = NominalCapacityDes;
                    } else if (this->finalSysSizing(this->curSysNum).CoolingCapMethod == DataSizing::CoolingDesignCapacity &&
                               this->finalSysSizing(this->curSysNum).CoolingTotalCapacity > 0.0) {
                        NominalCapacityDes = this->finalSysSizing(this->curSysNum).CoolingTotalCapacity;
                        this->autoSizedValue = NominalCapacityDes;
                    } else if (DesVolFlow >= DataHVACGlobals::SmallAirVolFlow) {
                        if (DesVolFlow > 0.0) {
                            OutAirFrac = this->finalSysSizing(this->curSysNum).DesOutAirVolFlow / DesVolFlow;
                        } else {
                            OutAirFrac = 1.0;
                        }
                        OutAirFrac = min(1.0, max(0.0, OutAirFrac));
                        if (this->curOASysNum > 0) { // coil is in the OA stream
                            CoilInTemp = this->finalSysSizing(this->curSysNum).OutTempAtCoolPeak;
                            CoilInHumRat = this->finalSysSizing(this->curSysNum).OutHumRatAtCoolPeak;
                            CoilOutTemp = this->finalSysSizing(this->curSysNum).PrecoolTemp;
                            CoilOutHumRat = this->finalSysSizing(this->curSysNum).PrecoolHumRat;
                        } else { // coil is on the main air loop
                            if (this->dataAirFlowUsedForSizing > 0.0) {
                                DesVolFlow = this->dataAirFlowUsedForSizing;
                            }
                            if (this->dataDesOutletAirTemp > 0.0) {
                                CoilOutTemp = this->dataDesOutletAirTemp;
                            } else {
                                CoilOutTemp = this->finalSysSizing(this->curSysNum).CoolSupTemp;
                            }
                            if (this->dataDesOutletAirHumRat > 0.0) {
                                CoilOutHumRat = this->dataDesOutletAirHumRat;
                            } else {
                                CoilOutHumRat = this->finalSysSizing(this->curSysNum).CoolSupHumRat;
                            }

                            if (this->primaryAirSystem(this->curSysNum).NumOACoolCoils == 0) { // there is no precooling of the OA stream
                                CoilInTemp = this->finalSysSizing(this->curSysNum).MixTempAtCoolPeak;
                                CoilInHumRat = this->finalSysSizing(this->curSysNum).MixHumRatAtCoolPeak;
                            } else { // there is precooling of OA stream
                                if (DesVolFlow > 0.0) {
                                    OutAirFrac = this->finalSysSizing(this->curSysNum).DesOutAirVolFlow / DesVolFlow;
                                } else {
                                    OutAirFrac = 1.0;
                                }
                                OutAirFrac = min(1.0, max(0.0, OutAirFrac));
                                CoilInTemp = OutAirFrac * this->finalSysSizing(this->curSysNum).PrecoolTemp +
                                             (1.0 - OutAirFrac) * this->finalSysSizing(this->curSysNum).RetTempAtCoolPeak;
                                CoilInHumRat = OutAirFrac * this->finalSysSizing(this->curSysNum).PrecoolHumRat +
                                               (1.0 - OutAirFrac) * this->finalSysSizing(this->curSysNum).RetHumRatAtCoolPeak;
                            }
                            if (this->dataDesInletAirTemp > 0.0) CoilInTemp = this->dataDesInletAirTemp;
                            if (this->dataDesInletAirHumRat > 0.0) CoilInHumRat = this->dataDesInletAirHumRat;
                        }
                        Real64 OutTemp = this->finalSysSizing(this->curSysNum).OutTempAtCoolPeak;
                        Real64 rhoair = 0.0;
                        if (UtilityRoutines::SameString(this->compType, "COIL:COOLING:WATER") ||
                            UtilityRoutines::SameString(this->compType, "COIL:COOLING:WATER:DETAILEDGEOMETRY")) {
                            rhoair = DataEnvironment::StdRhoAir;
                        } else {
                            rhoair = Psychrometrics::PsyRhoAirFnPbTdbW(DataEnvironment::StdBaroPress, CoilInTemp, CoilInHumRat, this->callingRoutine);
                        }
                        CoilOutTemp = min(CoilInTemp, CoilOutTemp);
                        CoilOutHumRat = min(CoilInHumRat, CoilOutHumRat);
                        Real64 CoilInEnth = Psychrometrics::PsyHFnTdbW(CoilInTemp, CoilInHumRat);
                        Real64 CoilInWetBulb =
                            Psychrometrics::PsyTwbFnTdbWPb(CoilInTemp, CoilInHumRat, DataEnvironment::StdBaroPress, this->callingRoutine);
                        Real64 CoilOutEnth = Psychrometrics::PsyHFnTdbW(CoilOutTemp, CoilOutHumRat);
                        if (this->curOASysNum > 0) { // coil is in the OA stream
                            // need to find fan type in OA system
                        } else {
                            switch (this->primaryAirSystem(this->curSysNum).supFanModelTypeEnum) {
                            case DataAirSystems::structArrayLegacyFanModels: {
                                FanCoolLoad = this->calcFanDesHeatGain(DesVolFlow);
                                break;
                            }
                            case DataAirSystems::objectVectorOOFanSystemModel: {
                                FanCoolLoad = this->calcFanDesHeatGain(DesVolFlow);
                                break;
                            }
                            case DataAirSystems::fanModelTypeNotYetSet: {
                                // do nothing
                                break;
                            }
                            } // end switch

                            switch (this->primaryAirSystem(this->curSysNum).retFanModelTypeEnum) {
                            case DataAirSystems::structArrayLegacyFanModels: {
                                FanCoolLoad += (1.0 - OutAirFrac) * this->calcFanDesHeatGain(DesVolFlow);
                                break;
                            }
                            case DataAirSystems::objectVectorOOFanSystemModel: {
                                FanCoolLoad += (1.0 - OutAirFrac) * this->calcFanDesHeatGain(DesVolFlow);
                                break;
                            }
                            case DataAirSystems::fanModelTypeNotYetSet: {
                                // do nothing
                                break;
                            }
                            } // end switch

                            this->primaryAirSystem(this->curSysNum).FanDesCoolLoad = FanCoolLoad;
                        }
                        Real64 PeakCoilLoad = max(0.0, (rhoair * DesVolFlow * (CoilInEnth - CoilOutEnth)));
                        Real64 CpAir = Psychrometrics::PsyCpAirFnW(CoilInHumRat);
                        // adjust coil inlet/outlet temp with fan temperature rise
                        if (this->dataDesAccountForFanHeat) {
                            PeakCoilLoad = max(0.0, (rhoair * DesVolFlow * (CoilInEnth - CoilOutEnth) + FanCoolLoad));
                            if (this->primaryAirSystem(this->curSysNum).supFanLocation == DataAirSystems::fanPlacement::BlowThru) {
                                CoilInTemp += FanCoolLoad / (CpAir * DataEnvironment::StdRhoAir * DesVolFlow);
                                // include change in inlet condition in TotCapTempModFac
                                CoilInWetBulb =
                                    Psychrometrics::PsyTwbFnTdbWPb(CoilInTemp, CoilInHumRat, DataEnvironment::StdBaroPress, this->callingRoutine);
                            } else if (this->primaryAirSystem(this->curSysNum).supFanLocation == DataAirSystems::fanPlacement::DrawThru) {
                                CoilOutTemp -= FanCoolLoad / (CpAir * DataEnvironment::StdRhoAir * DesVolFlow);
                            }
                        }
                        if (this->dataTotCapCurveIndex > 0) {
                            TotCapTempModFac = CurveManager::CurveValue(state, this->dataTotCapCurveIndex, CoilInWetBulb, OutTemp);
                        } else {
                            TotCapTempModFac = 1.0;
                        }
                        if (TotCapTempModFac > 0.0) {
                            NominalCapacityDes = PeakCoilLoad / TotCapTempModFac;
                        } else {
                            NominalCapacityDes = PeakCoilLoad;
                        }
                        DataSizing::DataCoilSizingAirInTemp = CoilInTemp;
                        DataSizing::DataCoilSizingAirInHumRat = CoilInHumRat;
                        DataSizing::DataCoilSizingAirOutTemp = CoilOutTemp;
                        DataSizing::DataCoilSizingAirOutHumRat = CoilOutHumRat;
                        DataSizing::DataCoilSizingFanCoolLoad = FanCoolLoad;
                        DataSizing::DataCoilSizingCapFT = TotCapTempModFac;
                    } else {
                        NominalCapacityDes = 0.0;
                    }
                    this->autoSizedValue =
                        NominalCapacityDes * this->dataFracOfAutosizedCoolingCapacity; // Fixed Moved up 1 line inside block per Richard Raustad
                }                                                                      // IF(OASysFlag) THEN or ELSE IF(AirLoopSysFlag) THEN
                this->dataDesAccountForFanHeat = true;                                 // reset for next water coil
                if (DataGlobals::DisplayExtraWarnings && this->autoSizedValue <= 0.0) {
                    ShowWarningMessage(this->callingRoutine + ": Potential issue with equipment sizing for " + this->compType + ' ' + this->compName);
                    ShowContinueError("...Rated Total Cooling Capacity = " + General::TrimSigDigits(this->autoSizedValue, 2) + " [W]");
                    if (this->oaSysFlag || this->airLoopSysFlag ||
                        this->finalSysSizing(this->curSysNum).CoolingCapMethod == DataSizing::CapacityPerFloorArea ||
                        (this->finalSysSizing(this->curSysNum).CoolingCapMethod == DataSizing::CoolingDesignCapacity &&
                         this->finalSysSizing(this->curSysNum).CoolingTotalCapacity)) {
                        ShowContinueError("...Capacity passed by parent object to size child component = " +
                                          General::TrimSigDigits(this->autoSizedValue, 2) + " [W]");
                    } else {
                        ShowContinueError("...Air flow rate used for sizing = " + General::TrimSigDigits(DesVolFlow, 5) + " [m3/s]");
                        ShowContinueError("...Outdoor air fraction used for sizing = " + General::TrimSigDigits(OutAirFrac, 2));
                        ShowContinueError("...Coil inlet air temperature used for sizing = " + General::TrimSigDigits(CoilInTemp, 2) + " [C]");
                        ShowContinueError("...Coil outlet air temperature used for sizing = " + General::TrimSigDigits(CoilOutTemp, 2) + " [C]");
                    }
                }
            }
        } else if (this->dataNonZoneNonAirloopValue > 0) {
            this->autoSizedValue = this->dataNonZoneNonAirloopValue;
        } else if (!this->wasAutoSized) {
            this->autoSizedValue = this->originalValue;
        } else {
            std::string msg = this->callingRoutine + ' ' + this->compType + ' ' + this->compName + ", Developer Error: Component sizing incomplete.";
            ShowSevereError(msg);
            this->addErrorMessage(msg);
            msg = "SizingString = " + this->sizingString + ", SizingResult = " + General::TrimSigDigits(this->autoSizedValue, 1);
            ShowContinueError(msg);
            this->addErrorMessage(msg);
            errorsFound = true;
        }
    }
    if (!this->hardSizeNoDesignRun || this->dataScalableSizingON || this->dataScalableCapSizingON) {
        if (this->wasAutoSized) {
            // Note: the VolFlowPerRatedTotCap check is not applicable for VRF-FluidTCtrl coil model, which implements variable flow fans and
            // determines capacity using physical calculations instead of emperical curves
            bool FlagCheckVolFlowPerRatedTotCap = true;
            if (UtilityRoutines::SameString(this->compType, "Coil:Cooling:DX:VariableRefrigerantFlow:FluidTemperatureControl") ||
                UtilityRoutines::SameString(this->compType, "Coil:Heating:DX:VariableRefrigerantFlow:FluidTemperatureControl"))
                FlagCheckVolFlowPerRatedTotCap = false;

            if (this->dataIsDXCoil && FlagCheckVolFlowPerRatedTotCap) {
                Real64 RatedVolFlowPerRatedTotCap = 0.0;
                if (this->autoSizedValue > 0.0) {
                    RatedVolFlowPerRatedTotCap = DesVolFlow / this->autoSizedValue;
                }
                if (RatedVolFlowPerRatedTotCap < DataHVACGlobals::MinRatedVolFlowPerRatedTotCap(DataHVACGlobals::DXCT)) {
                    if (!this->dataEMSOverride && DataGlobals::DisplayExtraWarnings && this->printWarningFlag) {
                        ShowWarningError(this->callingRoutine + ' ' + this->compType + ' ' + this->compName);
                        ShowContinueError("..." + this->sizingString +
                                          " will be limited by the minimum rated volume flow per rated total capacity ratio.");
                        ShowContinueError("...DX coil volume flow rate (m3/s ) = " + General::TrimSigDigits(DesVolFlow, 6));
                        ShowContinueError("...Requested capacity (W ) = " + General::TrimSigDigits(this->autoSizedValue, 3));
                        ShowContinueError("...Requested flow/capacity ratio (m3/s/W ) = " + General::TrimSigDigits(RatedVolFlowPerRatedTotCap, 3));
                        ShowContinueError("...Minimum flow/capacity ratio (m3/s/W ) = " +
                                          General::TrimSigDigits(DataHVACGlobals::MinRatedVolFlowPerRatedTotCap(DataHVACGlobals::DXCT), 3));
                    }

                    DXFlowPerCapMinRatio = (DesVolFlow / DataHVACGlobals::MinRatedVolFlowPerRatedTotCap(DataHVACGlobals::DXCT)) /
                                           this->autoSizedValue; // set DX Coil Capacity Increase Ratio from Too Low Flow/Capacity Ratio
                    this->autoSizedValue = DesVolFlow / DataHVACGlobals::MinRatedVolFlowPerRatedTotCap(DataHVACGlobals::DXCT);

                    if (!this->dataEMSOverride && DataGlobals::DisplayExtraWarnings && this->printWarningFlag) {
                        ShowContinueError("...Adjusted capacity ( W ) = " + General::TrimSigDigits(this->autoSizedValue, 3));
                    }
                } else if (RatedVolFlowPerRatedTotCap > DataHVACGlobals::MaxRatedVolFlowPerRatedTotCap(DataHVACGlobals::DXCT)) {
                    if (!this->dataEMSOverride && DataGlobals::DisplayExtraWarnings && this->printWarningFlag) {
                        ShowWarningError(this->callingRoutine + ' ' + this->compType + ' ' + this->compName);
                        ShowContinueError("..." + this->sizingString +
                                          " will be limited by the maximum rated volume flow per rated total capacity ratio.");
                        ShowContinueError("...DX coil volume flow rate ( m3/s ) = " + General::TrimSigDigits(DesVolFlow, 6));
                        ShowContinueError("...Requested capacity ( W ) = " + General::TrimSigDigits(this->autoSizedValue, 3));
                        ShowContinueError("...Requested flow/capacity ratio ( m3/s/W ) = " + General::TrimSigDigits(RatedVolFlowPerRatedTotCap, 3));
                        ShowContinueError("...Maximum flow/capacity ratio ( m3/s/W ) = " +
                                          General::TrimSigDigits(DataHVACGlobals::MaxRatedVolFlowPerRatedTotCap(DataHVACGlobals::DXCT), 3));
                    }

                    DXFlowPerCapMaxRatio = DesVolFlow / DataHVACGlobals::MaxRatedVolFlowPerRatedTotCap(DataHVACGlobals::DXCT) /
                                           this->autoSizedValue; // set DX Coil Capacity Decrease Ratio from Too High Flow/Capacity Ratio
                    this->autoSizedValue = DesVolFlow / DataHVACGlobals::MaxRatedVolFlowPerRatedTotCap(DataHVACGlobals::DXCT);

                    if (!this->dataEMSOverride && DataGlobals::DisplayExtraWarnings && this->printWarningFlag) {
                        ShowContinueError("...Adjusted capacity ( W ) = " + General::TrimSigDigits(this->autoSizedValue, 3));
                    }
                }
            }
        }
    }

    // override sizing string
    if (this->overrideSizeString) {
        if (this->isEpJSON) this->sizingString = "cooling_design_capacity [W]";
    }
    if (this->dataScalableCapSizingON) {
        auto const SELECT_CASE_var(this->zoneEqSizing(this->curZoneEqNum).SizingMethod(DataHVACGlobals::CoolingCapacitySizing));
        if (SELECT_CASE_var == DataSizing::CapacityPerFloorArea) {
            this->sizingStringScalable = "(scaled by capacity / area) ";
        } else if (SELECT_CASE_var == DataSizing::FractionOfAutosizedHeatingCapacity ||
                   SELECT_CASE_var == DataSizing::FractionOfAutosizedCoolingCapacity) {
            this->sizingStringScalable = "(scaled by fractional multiplier) ";
        }
    }

    this->selectSizerOutput(errorsFound);

    if (this->isCoilReportObject && this->curSysNum <= DataHVACGlobals::NumPrimaryAirSys) {
        if (CoilInTemp > -999.0) { // set inlet air properties used during capacity sizing if available, allow for negative winter temps
            coilSelectionReportObj->setCoilEntAirTemp(state, this->compName, this->compType, CoilInTemp, this->curSysNum, this->curZoneEqNum);
            coilSelectionReportObj->setCoilEntAirHumRat(this->compName, this->compType, CoilInHumRat);
        }
        if (CoilOutTemp > -999.0) { // set outlet air properties used during capacity sizing if available
            coilSelectionReportObj->setCoilLvgAirTemp(this->compName, this->compType, CoilOutTemp);
            coilSelectionReportObj->setCoilLvgAirHumRat(this->compName, this->compType, CoilOutHumRat);
        }
        coilSelectionReportObj->setCoilCoolingCapacity(state,
                                                       this->compName,
                                                       this->compType,
                                                       this->autoSizedValue,
                                                       this->wasAutoSized,
                                                       this->curSysNum,
                                                       this->curZoneEqNum,
                                                       this->curOASysNum,
                                                       FanCoolLoad,
                                                       TotCapTempModFac,
                                                       DXFlowPerCapMinRatio,
                                                       DXFlowPerCapMaxRatio);
    }
    return this->autoSizedValue;
}

void CoolingCapacitySizer::clearState()
{
    BaseSizerWithScalableInputs::clearState();
}

} // namespace EnergyPlus
