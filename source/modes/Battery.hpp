class BatteryOverlay : public tsl::Gui {
private:
	char Battery_c[512];
public:
    BatteryOverlay() {
		mutexInit(&mutex_BatteryChecker);
		StartBatteryThread();
	}
	~BatteryOverlay() {
		CloseThreads();
	}

    virtual tsl::elm::Element* createUI() override {
		rootFrame = new tsl::elm::OverlayFrame("상태 모니터", APP_VERSION"-ASAP");

		auto Status = new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer *renderer, u16 x, u16 y, u16 w, u16 h) {
			renderer->drawString("배터리/충전 상태", false, 20, 120, 20, renderer->a(tsl::statusTextColor));
			renderer->drawString(Battery_c, false, 20, 175, 18, renderer->a(tsl::defaultTextColor));
		});

		rootFrame->setContent(Status);

		return rootFrame;
	}

	virtual void update() override {

		///Battery

		mutexLock(&mutex_BatteryChecker);
		char tempBatTimeEstimate[8] = "-:--";
		if (batTimeEstimate >= 0) {
			snprintf(&tempBatTimeEstimate[0], sizeof(tempBatTimeEstimate), "%d:%02d", batTimeEstimate / 60, batTimeEstimate % 60);
		}

		BatteryChargeInfoFieldsChargerType ChargerConnected = _batteryChargeInfoFields.ChargerType;
		int32_t ChargerVoltageLimit = _batteryChargeInfoFields.ChargerVoltageLimit;
		int32_t ChargerCurrentLimit = _batteryChargeInfoFields.ChargerCurrentLimit;

		if (hosversionAtLeast(17,0,0)) {
			ChargerConnected = ((BatteryChargeInfoFields17*)&_batteryChargeInfoFields) -> ChargerType;
			ChargerVoltageLimit = ((BatteryChargeInfoFields17*)&_batteryChargeInfoFields) -> ChargerVoltageLimit;
			ChargerCurrentLimit = ((BatteryChargeInfoFields17*)&_batteryChargeInfoFields) -> ChargerCurrentLimit;
		}

		if (ChargerConnected)
			snprintf(Battery_c, sizeof Battery_c,
				"실제 용량 : %.0f mAh\n"
				"설계 용량 : %.0f mAh\n\n"
				"현재 온도 : %.1f\u00B0C\n\n"
				"현재 잔량 : %.1f%%\n"
				"예상 수명 : %.1f%%\n\n"
				"현재 전압 (%d초 평균) : %.0f mV\n"
				"현재 전류 (%s초 평균) : %+.0f mA\n"
				"소비 전력 %s : %+.3f W\n\n"
				"잔여 시간 : %s\n\n"
				"입력 전류 제한: %d mA\n"
				"VBUS 전류 제한: %d mA\n" 
				"충전 전압 제한: %d mV\n"
				"충전 전류 제한: %d mA\n\n"
				"충전 타입 : %u\n\n"
				"충전 최대 전압 : %u mV\n"
				"충전 최대 전류 : %u mA",
				actualFullBatCapacity,
				designedFullBatCapacity,
				(float)_batteryChargeInfoFields.BatteryTemperature / 1000,
				(float)_batteryChargeInfoFields.RawBatteryCharge / 1000,
				(float)_batteryChargeInfoFields.BatteryAge / 1000,
				batteryFiltered ? 45 : 5, batVoltageAvg,
				batteryFiltered ? "11.25" : "5", batCurrentAvg,
				batteryFiltered ? "" : "(5초 평균)", PowerConsumption, 
				tempBatTimeEstimate,
				_batteryChargeInfoFields.InputCurrentLimit,
				_batteryChargeInfoFields.VBUSCurrentLimit,
				_batteryChargeInfoFields.ChargeVoltageLimit,
				_batteryChargeInfoFields.ChargeCurrentLimit,
				ChargerConnected,
				ChargerVoltageLimit,
				ChargerCurrentLimit
			);
		else
			snprintf(Battery_c, sizeof Battery_c,
				"실제 용량 : %.0f mAh\n"
				"설계 용량 : %.0f mAh\n\n"
				"현재 온도 : %.1f\u00B0C\n\n"
				"현재 잔량 : %.1f%%\n"
				"예상 수명 : %.1f%%\n\n"
				"현재 전압 (%d초 평균) : %.0f mV\n"
				"현재 전류 (%s초 평균) : %+.0f mA\n"
				"소비 전력 %s : %+.3f W\n\n"
				"잔여 시간 : %s",
				actualFullBatCapacity,
				designedFullBatCapacity,
				(float)_batteryChargeInfoFields.BatteryTemperature / 1000,
				(float)_batteryChargeInfoFields.RawBatteryCharge / 1000,
				(float)_batteryChargeInfoFields.BatteryAge / 1000,
				batteryFiltered ? 45 : 5, batVoltageAvg,
				batteryFiltered ? "11.25" : "5", batCurrentAvg,
				batteryFiltered ? "" : "(5초 평균)", PowerConsumption, 
				tempBatTimeEstimate
			);
		mutexUnlock(&mutex_BatteryChecker);
		
	}
	virtual bool handleInput(uint64_t keysDown, uint64_t keysHeld, touchPosition touchInput, JoystickPosition leftJoyStick, JoystickPosition rightJoyStick) override {
		if (keysDown & KEY_B) {
			tsl::goBack();
			return true;
		}
		return false;
	}
};
