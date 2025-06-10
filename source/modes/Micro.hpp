class MicroOverlay : public tsl::Gui {
private:
	uint64_t mappedButtons = MapButtons(keyCombo); // map buttons
	char GPU_Load_c[32] = "";
	char Rotation_SpeedLevel_c[64] = "";
	char RAM_var_compressed_c[128] = "";
	char CPU_compressed_c[160] = "";
	char CPU_Usage0[32] = "";
	char CPU_Usage1[32] = "";
	char CPU_Usage2[32] = "";
	char CPU_Usage3[32] = "";
	char CPU_UsageM[32] = "";
	char SOC_temp_c[16] = "";
	char PCB_temp_c[16] = "";
	char SKIN_temp_c[16] = "";	
	char batteryCharge[10] = ""; // Declare the batteryCharge variable
	char FPS_var_compressed_c[64] = "";
	char Power_c[16];
	char Battery_c[32];
	char CPU_volt_c[16];
	char GPU_volt_c[16];
	char RAM_volt_c[32];
	char SOC_volt_c[16];

	const uint32_t margin = 4;

	bool Initialized = false;
	MicroSettings settings;
	size_t text_width = 0;
	size_t fps_width = 0;
	ApmPerformanceMode performanceMode = ApmPerformanceMode_Invalid;
	size_t fontsize = 0;
	bool showFPS = false;
	uint64_t systemtickfrequency_impl = systemtickfrequency;
public:
    MicroOverlay() { 
		GetConfigSettings(&settings);
		apmGetPerformanceMode(&performanceMode);
		if (performanceMode == ApmPerformanceMode_Normal) {
			fontsize = settings.handheldFontSize;
		}
		else fontsize = settings.dockedFontSize;
		if (settings.setPosBottom) {
			tsl::gfx::Renderer::getRenderer().setLayerPos(0, 1038);
		}
		mutexInit(&mutex_BatteryChecker);
		mutexInit(&mutex_Misc);
		TeslaFPS = settings.refreshRate;
		systemtickfrequency_impl /= settings.refreshRate;
		alphabackground = 0x0;
		deactivateOriginalFooter = true;
        StartThreads();
	}
	~MicroOverlay() {
		CloseThreads();
	}
    
    virtual tsl::elm::Element* createUI() override {
		rootFrame = new tsl::elm::OverlayFrame("", "");

		auto Status = new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer *renderer, u16 x, u16 y, u16 w, u16 h) {

			if (!Initialized) {
				Initialized = true;
				tsl::hlp::requestForeground(false);
			}

			u32 base_y = 1;
			/*if (settings.setPosBottom) {
				base_y = tsl::cfg::FramebufferHeight - (fontsize + (fontsize / 4));
			}*/

			renderer->drawRect(0, base_y, tsl::cfg::FramebufferWidth, fontsize + (fontsize / 4), renderer->a(tsl::defaultBackgroundColor));

			uint32_t offset = 0;
			uint8_t flags = 0;
			for (std::string key : ult::split(settings.show, '+')) {
				if (!key.compare("CPU") && !(flags & 1 << 0)) {
					auto dimensions_s = renderer->drawString("CPU", false, offset, base_y+fontsize, fontsize, renderer->a(tsl::statusTextColor));
					offset += dimensions_s.first + margin;
					auto dimensions_d = renderer->drawString(CPU_compressed_c, false, offset, base_y+fontsize, fontsize, renderer->a(tsl::defaultTextColor));
					offset += dimensions_d.first + margin;
					if (settings.realVolts) {
						auto dimensions_e = renderer->drawString("|", false, offset, base_y+fontsize, fontsize, renderer->a(tsl::defaultTextColor));
						offset += dimensions_e.first + margin;
					}
					if (settings.realVolts) {
						auto dimensions_v = renderer->drawString(CPU_volt_c, false, offset, base_y+fontsize, fontsize, renderer->a(tsl::defaultTextColor));
						offset += dimensions_v.first + margin;
					}
					offset += margin;
					flags |= 1 << 0;
				}
				else if (!key.compare("GPU") && !(flags & 1 << 1)) {
					auto dimensions_s = renderer->drawString("GPU", false, settings.setPosGPU, base_y+fontsize, fontsize, renderer->a(tsl::statusTextColor));
					offset = settings.setPosGPU + dimensions_s.first + margin;
					auto dimensions_d = renderer->drawString(GPU_Load_c, false, offset, base_y+fontsize, fontsize, renderer->a(tsl::defaultTextColor));
					offset += dimensions_d.first + margin;
					if (settings.realVolts) {
						auto dimensions_e = renderer->drawString("|", false, offset, base_y+fontsize, fontsize, renderer->a(tsl::defaultTextColor));
						offset += dimensions_e.first + margin;
					}
					if (settings.realVolts) {
						auto dimensions_v = renderer->drawString(GPU_volt_c, false, offset, base_y+fontsize, fontsize, renderer->a(tsl::defaultTextColor));
						offset += dimensions_v.first + margin;
					}
					offset += margin;
					flags |= 1 << 1;
				}
				else if (!key.compare("RAM") && !(flags & 1 << 2)) {
					auto dimensions_s = renderer->drawString("RAM", false, settings.setPosRAM, base_y+fontsize, fontsize, renderer->a(tsl::statusTextColor));
					offset = settings.setPosRAM + dimensions_s.first + margin;
					auto dimensions_d = renderer->drawString(RAM_var_compressed_c, false, offset, base_y+fontsize, fontsize, renderer->a(tsl::defaultTextColor));
					offset += dimensions_d.first + margin;
					if (settings.realVolts) {
						auto dimensions_e = renderer->drawString("|", false, offset, base_y+fontsize, fontsize, renderer->a(tsl::defaultTextColor));
						offset += dimensions_e.first + margin;
					}
					if (settings.realVolts) {
						auto dimensions_v = renderer->drawString(RAM_volt_c, false, offset, base_y+fontsize, fontsize, renderer->a(tsl::defaultTextColor));
						offset += dimensions_v.first + margin;
					}
					offset += margin;
					flags |= 1 << 2;
				}
				else if (!key.compare("SOC") && !(flags & 1 << 3)) {
					auto dimensions_s = renderer->drawString("SoC", false, settings.setPosSOC, base_y+fontsize, fontsize, renderer->a(tsl::statusTextColor));
					offset = settings.setPosSOC + dimensions_s.first + margin;
					auto dimensions_d = renderer->drawString(SOC_temp_c, false, offset, base_y+fontsize, fontsize, renderer->a(tsl::defaultTextColor));
					offset += dimensions_d.first + margin;
					if (settings.realVolts) {
						auto dimensions_e = renderer->drawString("|", false, offset, base_y+fontsize, fontsize, renderer->a(tsl::defaultTextColor));
						offset += dimensions_e.first + margin;
						auto dimensions_v = renderer->drawString(SOC_volt_c, false, offset, base_y+fontsize, fontsize, renderer->a(tsl::defaultTextColor));
						offset += dimensions_v.first + margin;
					}
					offset += margin;
					flags |= 1 << 3;
				}
				else if (!key.compare("PCB") && !(flags & 1 << 4)) {
					auto dimensions_s = renderer->drawString("PCB", false, settings.setPosPCB, base_y+fontsize, fontsize, renderer->a(tsl::statusTextColor));
					offset = settings.setPosPCB + dimensions_s.first + margin;
					auto dimensions_d = renderer->drawString(PCB_temp_c, false, offset, base_y+fontsize, fontsize, renderer->a(tsl::defaultTextColor));
					offset += dimensions_d.first + margin;
					offset += margin;
					flags |= 1 << 4;
				}
				else if (!key.compare("SKIN") && !(flags & 1 << 5)) {
					auto dimensions_s = renderer->drawString("Skin", false, settings.setPosSKIN, base_y+fontsize, fontsize, renderer->a(tsl::statusTextColor));
					offset = settings.setPosSKIN + dimensions_s.first + margin;
					auto dimensions_d = renderer->drawString(SKIN_temp_c, false, offset, base_y+fontsize, fontsize, renderer->a(tsl::defaultTextColor));
					offset += dimensions_d.first + margin;
					offset += margin;
					flags |= 1 << 5;
				}
				else if (!key.compare("FAN") && !(flags & 1 << 6)) {
					auto dimensions_s = renderer->drawString("FAN", false, settings.setPosFAN, base_y+fontsize, fontsize, renderer->a(tsl::statusTextColor));
					offset = settings.setPosFAN + dimensions_s.first + margin;
					renderer->drawString(Rotation_SpeedLevel_c, false, offset, base_y+fontsize, fontsize, renderer->a(tsl::defaultTextColor));
					offset += margin;
					flags |= 1 << 6;
				}
				else if (!key.compare("PWR") && !(flags & 1 << 7)) {
					auto dimensions_s = renderer->drawString("PWR", false, 1095, base_y+fontsize, fontsize, renderer->a(tsl::statusTextColor));
					offset = 1095 + dimensions_s.first + margin;
					auto dimensions_d = renderer->drawString(Power_c, false, offset, base_y+fontsize, fontsize, renderer->a(tsl::defaultTextColor));
					offset += dimensions_d.first + margin;
					/*auto dimensions_v = renderer->drawString("|", false, offset, base_y+fontsize, fontsize, renderer->a(tsl::statusTextColor));
					offset += dimensions_v.first + margin;*/
					flags |= 1 << 7;
				}
				else if (!key.compare("BAT") && !(flags & 1 << 8)) {
					auto dimensions_d = renderer->drawString(Battery_c, false, offset, base_y+fontsize, fontsize, renderer->a(0x0000));
					renderer->drawString(Battery_c, false, tsl::cfg::FramebufferWidth - dimensions_d.first, base_y+fontsize, fontsize, renderer->a(settings.batColor));
					flags |= 1 << 8;
				}
				else if (!key.compare("FPS") && !(flags & 1 << 9)) {
					auto dimensions_s = renderer->drawString(" FPS", false, offset, base_y+fontsize, fontsize, renderer->a(tsl::statusTextColor));
					uint32_t offset_s = offset + dimensions_s.first + margin;
					renderer->drawString(FPS_var_compressed_c, false, offset_s, base_y+fontsize, fontsize, renderer->a(tsl::defaultTextColor));
					offset += settings.setPosCPU;
					flags |= 1 << 9;
				}
			}
		});

		rootFrame->setContent(Status);

		return rootFrame;
	}

	virtual void update() override {
		apmGetPerformanceMode(&performanceMode);
		if (performanceMode == ApmPerformanceMode_Normal) {
			if (fontsize != settings.handheldFontSize) {
				Initialized = false;
				fontsize = settings.handheldFontSize;
			}
		}
		else if (performanceMode == ApmPerformanceMode_Boost) {
			if (fontsize != settings.dockedFontSize) {
				Initialized = false;
				fontsize = settings.dockedFontSize;
			}
		}

		//Make stuff ready to print
		///CPU
		if (idletick0 > systemtickfrequency_impl) idletick0 = systemtickfrequency_impl;
		if (idletick1 > systemtickfrequency_impl) idletick1 = systemtickfrequency_impl;
		if (idletick2 > systemtickfrequency_impl) idletick2 = systemtickfrequency_impl;
		if (idletick3 > systemtickfrequency_impl) idletick3 = systemtickfrequency_impl;
		double cpu_usage0 = (1.d - ((double)idletick0 / systemtickfrequency_impl)) * 100;
		double cpu_usage1 = (1.d - ((double)idletick1 / systemtickfrequency_impl)) * 100;
		double cpu_usage2 = (1.d - ((double)idletick2 / systemtickfrequency_impl)) * 100;
		double cpu_usage3 = (1.d - ((double)idletick3 / systemtickfrequency_impl)) * 100;
		double cpu_usageM = 0;
		if (cpu_usage0 > cpu_usageM)	cpu_usageM = cpu_usage0;
		if (cpu_usage1 > cpu_usageM)	cpu_usageM = cpu_usage1;
		if (cpu_usage2 > cpu_usageM)	cpu_usageM = cpu_usage2;
		if (cpu_usage3 > cpu_usageM)	cpu_usageM = cpu_usage3;
		snprintf(CPU_Usage0, sizeof CPU_Usage0, "%.0f%%", cpu_usage0);
		snprintf(CPU_Usage1, sizeof CPU_Usage1, "%.0f%%", cpu_usage1);
		snprintf(CPU_Usage2, sizeof CPU_Usage2, "%.0f%%", cpu_usage2);
		snprintf(CPU_Usage3, sizeof CPU_Usage3, "%.0f%%", cpu_usage3);
		snprintf(CPU_UsageM, sizeof CPU_UsageM, "%.0f%%", cpu_usageM);

		mutexLock(&mutex_Misc);
		char difference[5] = "@";
		if (realCPU_Hz) {
			int32_t deltaCPU = (int32_t)(realCPU_Hz / 1000) - (CPU_Hz / 1000);
			if (deltaCPU >= 25500) {
				strcpy(difference, "△");
			}
			else if (deltaCPU >= 102000 || deltaCPU <= -102000) {
				strcpy(difference, "≠");
			}
			else if (deltaCPU <= -25500) {
				strcpy(difference, "▽");
			}
		}
		if (settings.realFrequencies && realCPU_Hz) {
			if (settings.showFullCPU) {
				snprintf(CPU_compressed_c, sizeof CPU_compressed_c, 
				"[%s,%s,%s,%s]%s%d.%d", 
				CPU_Usage0, CPU_Usage1, CPU_Usage2, CPU_Usage3, 
				difference, 
				realCPU_Hz / 1000000, (realCPU_Hz / 100000) % 10);
			}
			else {
				snprintf(CPU_compressed_c, sizeof CPU_compressed_c, 
				"%s%s%d.%d", 
				CPU_UsageM, 
				difference, 
				realCPU_Hz / 1000000, (realCPU_Hz / 100000) % 10);
			}
		}
		else {
			if (settings.showFullCPU) {
				snprintf(CPU_compressed_c, sizeof CPU_compressed_c, 
				"[%s,%s,%s,%s]%s%d.%d", 
				CPU_Usage0, CPU_Usage1, CPU_Usage2, CPU_Usage3, 
				difference, 
				CPU_Hz / 1000000, (CPU_Hz / 100000) % 10);
			}
			else {
				snprintf(CPU_compressed_c, sizeof CPU_compressed_c, 
				"%s%s%d.%d", 
				CPU_UsageM, 
				difference, 
				CPU_Hz / 1000000, (CPU_Hz / 100000) % 10);
			}
		}
		if (settings.realVolts) {
			if (isMariko) {
				snprintf(CPU_volt_c, sizeof(CPU_volt_c), "%u.%u mV", realCPU_mV/1000, (realCPU_mV/100)%10);
			}
			else {
				snprintf(CPU_volt_c, sizeof(CPU_volt_c), "%u.%u mV", realCPU_mV/1000, (realCPU_mV/10)%100);
			} 
		}
		
		///GPU
		if (realGPU_Hz) {
			int32_t deltaGPU = (int32_t)(realGPU_Hz / 1000) - (GPU_Hz / 1000);
			if (deltaGPU) {
				strcpy(difference, "≠");
			}
			else {
				strcpy(difference, "@");
			}
		}
		else {
			strcpy(difference, "@");
		}
		if (settings.realFrequencies && realGPU_Hz) {
			snprintf(GPU_Load_c, sizeof GPU_Load_c, 
				"%d.%d%%%s%d.%d", 
				GPU_Load_u / 10, GPU_Load_u % 10, 
				difference, 
				realGPU_Hz / 1000000, (realGPU_Hz / 100000) % 10);
		}
		else {
			snprintf(GPU_Load_c, sizeof GPU_Load_c, 
				"%d.%d%%%s%d.%d", 
				GPU_Load_u / 10, GPU_Load_u % 10, 
				difference, 
				GPU_Hz / 1000000, (GPU_Hz / 100000) % 10);
		}
		if (settings.realVolts) {
			if (isMariko) {
				snprintf(GPU_volt_c, sizeof(GPU_volt_c), "%u.%u mV", realGPU_mV/1000, (realGPU_mV/100)%10);
			}
			else {
				snprintf(GPU_volt_c, sizeof(GPU_volt_c), "%u.%u mV", realGPU_mV/1000, (realGPU_mV/10)%100);
			}
		}
		
		///RAM
		char MICRO_RAM_all_c[12] = "";
		if (!settings.showRAMLoad || R_FAILED(sysclkCheck)) {
			float RAM_Total_application_f = (float)RAM_Total_application_u / 1024 / 1024;
			float RAM_Total_applet_f = (float)RAM_Total_applet_u / 1024 / 1024;
			float RAM_Total_system_f = (float)RAM_Total_system_u / 1024 / 1024;
			float RAM_Total_systemunsafe_f = (float)RAM_Total_systemunsafe_u / 1024 / 1024;
			float RAM_Total_all_f = RAM_Total_application_f + RAM_Total_applet_f + RAM_Total_system_f + RAM_Total_systemunsafe_f;
			float RAM_Used_application_f = (float)RAM_Used_application_u / 1024 / 1024;
			float RAM_Used_applet_f = (float)RAM_Used_applet_u / 1024 / 1024;
			float RAM_Used_system_f = (float)RAM_Used_system_u / 1024 / 1024;
			float RAM_Used_systemunsafe_f = (float)RAM_Used_systemunsafe_u / 1024 / 1024;
			float RAM_Used_all_f = RAM_Used_application_f + RAM_Used_applet_f + RAM_Used_system_f + RAM_Used_systemunsafe_f;
			snprintf(MICRO_RAM_all_c, sizeof(MICRO_RAM_all_c), "%.1f/%.1fGB", RAM_Used_all_f/1024, RAM_Total_all_f/1024);
		}
		else {
			snprintf(MICRO_RAM_all_c, sizeof(MICRO_RAM_all_c), "%hu.%hhu%%", ramLoad[SysClkRamLoad_All] / 10, ramLoad[SysClkRamLoad_All] % 10);
		}

		if (realRAM_Hz) {
			int32_t deltaRAM = (int32_t)(realRAM_Hz / 1000) - (RAM_Hz / 1000);
			if (deltaRAM <= -33000 || deltaRAM >= 33000) {
				strcpy(difference, "≠");
			}
			else if (deltaRAM >= 8250) {
				strcpy(difference, "△");
			}
			else if (deltaRAM > -8250) {
				strcpy(difference, "@");
			}
			else if (deltaRAM <= -8250) {
				strcpy(difference, "▽");
			}
		}
		else {
			strcpy(difference, "@");
		}
		if (settings.realFrequencies && realRAM_Hz) {
			snprintf(RAM_var_compressed_c, sizeof RAM_var_compressed_c, 
				"%s%s%d.%d", 
				MICRO_RAM_all_c, difference, realRAM_Hz / 1000000, (realRAM_Hz / 100000) % 10);
		}
		else {
			snprintf(RAM_var_compressed_c, sizeof RAM_var_compressed_c, 
				"%s%s%d.%d", 
				MICRO_RAM_all_c, difference, RAM_Hz / 1000000, (RAM_Hz / 1000000) % 10);
		}
		if (settings.realVolts) {
			uint32_t vdd2 = realRAM_mV / 10000;
            uint32_t vddq = realRAM_mV % 10000;
			if (isMariko) {
				snprintf(RAM_volt_c, sizeof(RAM_volt_c), "%u.%u/%u.%u mV", vdd2/10, vdd2%10, vddq/10, vddq%10);
			}
			else {
				snprintf(RAM_volt_c, sizeof(RAM_volt_c), "%u.%u mV", vdd2/10, vdd2%10);
			} 
		}

		char remainingBatteryLife[8];
		mutexLock(&mutex_BatteryChecker);
		if (batTimeEstimate >= 0) {
			snprintf(remainingBatteryLife, sizeof remainingBatteryLife, "%d:%02d", batTimeEstimate / 60, batTimeEstimate % 60);
		}
		else snprintf(remainingBatteryLife, sizeof remainingBatteryLife, "--:--");

		snprintf(Power_c, sizeof Power_c, "%+.2fW ", PowerConsumption);
		snprintf(Battery_c, sizeof Battery_c, "%.1f%s [%s] ", (float)_batteryChargeInfoFields.RawBatteryCharge / 1000, "%", remainingBatteryLife);

		///Thermal
		snprintf(SOC_temp_c, sizeof SOC_temp_c, "%2.1f\u00B0C", SOC_temperatureF);
		snprintf(PCB_temp_c, sizeof PCB_temp_c, "%2.1f\u00B0C", PCB_temperatureF);
		snprintf(SKIN_temp_c, sizeof SKIN_temp_c, "%hu.%hhu\u00B0C", skin_temperaturemiliC / 1000, (skin_temperaturemiliC / 100) % 10);

		mutexUnlock(&mutex_BatteryChecker);
		snprintf(Rotation_SpeedLevel_c, sizeof Rotation_SpeedLevel_c, "%2.0f%%", Rotation_Duty);

		if (settings.realVolts) {
			snprintf(SOC_volt_c, sizeof(SOC_volt_c), "%u.%u mV", realSOC_mV/1000, (realSOC_mV/100)%10);
		}

		///FPS
		snprintf(FPS_var_compressed_c, sizeof FPS_var_compressed_c, "%2.0f", FPSavg);

		mutexUnlock(&mutex_Misc);
	}

	virtual bool handleInput(uint64_t keysDown, uint64_t keysHeld, touchPosition touchInput, JoystickPosition leftJoyStick, JoystickPosition rightJoyStick) override {
		if (isKeyComboPressed(keysHeld, keysDown, mappedButtons)) {
			TeslaFPS = 60;
            if (skipMain)
                tsl::goBack();
            else {
			    tsl::setNextOverlay(filepath.c_str());
			    tsl::Overlay::get()->close();
            }
			return true;
		}
		return false;
	}
};