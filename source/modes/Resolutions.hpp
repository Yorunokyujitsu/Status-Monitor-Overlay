class ResolutionsOverlay : public tsl::Gui {
private:
	uint64_t mappedButtons = MapButtons(keyCombo);
	char Resolutions_c[512];
	char Resolutions2_c[512];
	ResolutionSettings settings;
public:
    ResolutionsOverlay() {
		GetConfigSettings(&settings);
		switch(settings.setPos) {
			case 1:
			case 4:
			case 7:
				tsl::gfx::Renderer::getRenderer().setLayerPos(639, 0);
				break;
			case 2:
			case 5:
			case 8:
				tsl::gfx::Renderer::getRenderer().setLayerPos(1248, 0);
				break;
		}
		StartFPSCounterThread();
		deactivateOriginalFooter = true;
		tsl::hlp::requestForeground(false);
		alphabackground = 0x0;
		FullMode = false;
		TeslaFPS = settings.refreshRate;
	}
	~ResolutionsOverlay() {
		TeslaFPS = 60;
		EndFPSCounterThread();
		deactivateOriginalFooter = false;
		tsl::hlp::requestForeground(true);
		alphabackground = 0xD;
		FullMode = true;
		if (settings.setPos)
			tsl::gfx::Renderer::getRenderer().setLayerPos(0, 0);
	}

	resolutionCalls m_resolutionRenderCalls[8] = {0};
	resolutionCalls m_resolutionViewportCalls[8] = {0};
	bool gameStart = false;
	uint8_t resolutionLookup = 0;

    virtual tsl::elm::Element* createUI() override {
		rootFrame = new tsl::elm::OverlayFrame("", "");
		
		auto Status = new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer *renderer, u16 x, u16 y, u16 w, u16 h) {
			int base_y = 0;
			int base_x = 0;
			const int frameWidth = 448;
		
			switch(settings.setPos) {
				case 1:
					base_x = 48;
					break;
				case 2:
					base_x = 96;
					break;
				case 3:
					base_y = 260;
					break;
				case 4:
					base_x = 48;
					base_y = 260;
					break;
				case 5:
					base_x = 96;
					base_y = 260;
					break;
				case 6:
					base_y = 520;
					break;
				case 7:
					base_x = 48;
					base_y = 520;
					break;
				case 8:
					base_x = 96;
					base_y = 520;
					break;	
			}

			// Adjust for right-side alignment
			if (ult::useRightAlignment) {
				base_x = frameWidth - base_x - 360; // Subtract width of the box (360px) from the frame width
			}

			if (gameStart && NxFps -> API == 1) {
				renderer->drawRect(base_x, base_y, 360, 200, renderer->a(tsl::defaultBackgroundColor));

				renderer->drawString("심도 [Depth]:", false, base_x+20, base_y+20, 20, renderer->a(tsl::statusTextColor));
				renderer->drawString(Resolutions_c, false, base_x+20, base_y+55, 18, renderer->a(tsl::defaultTextColor));
				renderer->drawString("노출 [Viewport]:", false, base_x+180, base_y+20, 20, renderer->a(tsl::statusTextColor));
				renderer->drawString(Resolutions2_c, false, base_x+180, base_y+55, 18, renderer->a(tsl::defaultTextColor));
			}
			else {
				switch(settings.setPos) {
					case 3 ... 5:
						base_y = 346;
						break;
					case 6 ... 8:
						base_y = 692;
						break;
				}
				if (gameStart && NxFps -> API > 1) {
					renderer->drawRect(base_x, base_y, 360, 28, renderer->a(tsl::defaultBackgroundColor));
					renderer->drawString("  - NVN을 사용하지 않아 호환되지 않습니다.", false, base_x, base_y+20, 18, renderer->a(tsl::warningTextColor));		
				}
				else {
					renderer->drawRect(base_x, base_y, 360, 28, renderer->a(tsl::defaultBackgroundColor));
					renderer->drawString("  - 실행중이거나 호환되는 게임이 없습니다.", false, base_x, base_y+20, 18, renderer->a(tsl::warningTextColor));
				}
			}
		});

		rootFrame->setContent(Status);

		return rootFrame;
	}

	virtual void update() override {

		if (gameStart && NxFps) {
			if (!resolutionLookup) {
				NxFps -> renderCalls[0].calls = 0xFFFF;
				resolutionLookup = 1;
			}
			else if (resolutionLookup == 1) {
				if ((NxFps -> renderCalls[0].calls) != 0xFFFF) resolutionLookup = 2;
				else return;
			}
			memcpy(&m_resolutionRenderCalls, &(NxFps -> renderCalls), sizeof(m_resolutionRenderCalls));
			memcpy(&m_resolutionViewportCalls, &(NxFps -> viewportCalls), sizeof(m_resolutionViewportCalls));
			qsort(m_resolutionRenderCalls, 8, sizeof(resolutionCalls), compare);
			qsort(m_resolutionViewportCalls, 8, sizeof(resolutionCalls), compare);
			snprintf(Resolutions_c, sizeof Resolutions_c,
				"1. %dx%d, %d\n"
				"2. %dx%d, %d\n"
				"3. %dx%d, %d\n"
				"4. %dx%d, %d\n"
				"5. %dx%d, %d\n"
				"6. %dx%d, %d\n"
				"7. %dx%d, %d\n"
				"8. %dx%d, %d",
				m_resolutionRenderCalls[0].width, m_resolutionRenderCalls[0].height, m_resolutionRenderCalls[0].calls,
				m_resolutionRenderCalls[1].width, m_resolutionRenderCalls[1].height, m_resolutionRenderCalls[1].calls,
				m_resolutionRenderCalls[2].width, m_resolutionRenderCalls[2].height, m_resolutionRenderCalls[2].calls,
				m_resolutionRenderCalls[3].width, m_resolutionRenderCalls[3].height, m_resolutionRenderCalls[3].calls,
				m_resolutionRenderCalls[4].width, m_resolutionRenderCalls[4].height, m_resolutionRenderCalls[4].calls,
				m_resolutionRenderCalls[5].width, m_resolutionRenderCalls[5].height, m_resolutionRenderCalls[5].calls,
				m_resolutionRenderCalls[6].width, m_resolutionRenderCalls[6].height, m_resolutionRenderCalls[6].calls,
				m_resolutionRenderCalls[7].width, m_resolutionRenderCalls[7].height, m_resolutionRenderCalls[7].calls
			);
			snprintf(Resolutions2_c, sizeof Resolutions2_c,
				"%dx%d, %d\n"
				"%dx%d, %d\n"
				"%dx%d, %d\n"
				"%dx%d, %d\n"
				"%dx%d, %d\n"
				"%dx%d, %d\n"
				"%dx%d, %d\n"
				"%dx%d, %d",
				m_resolutionViewportCalls[0].width, m_resolutionViewportCalls[0].height, m_resolutionViewportCalls[0].calls,
				m_resolutionViewportCalls[1].width, m_resolutionViewportCalls[1].height, m_resolutionViewportCalls[1].calls,
				m_resolutionViewportCalls[2].width, m_resolutionViewportCalls[2].height, m_resolutionViewportCalls[2].calls,
				m_resolutionViewportCalls[3].width, m_resolutionViewportCalls[3].height, m_resolutionViewportCalls[3].calls,
				m_resolutionViewportCalls[4].width, m_resolutionViewportCalls[4].height, m_resolutionViewportCalls[4].calls,
				m_resolutionViewportCalls[5].width, m_resolutionViewportCalls[5].height, m_resolutionViewportCalls[5].calls,
				m_resolutionViewportCalls[6].width, m_resolutionViewportCalls[6].height, m_resolutionViewportCalls[6].calls,
				m_resolutionViewportCalls[7].width, m_resolutionViewportCalls[7].height, m_resolutionViewportCalls[7].calls
			);
		}
		if (FPSavg < 254) {
			gameStart = true;
		}
		else {
			gameStart = false;
			resolutionLookup = false;
		}
	}
	virtual bool handleInput(uint64_t keysDown, uint64_t keysHeld, touchPosition touchInput, JoystickPosition leftJoyStick, JoystickPosition rightJoyStick) override {
		if (isKeyComboPressed(keysHeld, keysDown, mappedButtons)) {
			tsl::goBack();
			return true;
		}
		return false;
	}
};