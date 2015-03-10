/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/
#ifndef PINPADDIALOG_H
#define	PINPADDIALOG_H

#include "PinDialog.h"
#include <thread>

#define COUNTDOWN_STEPS 30

class PinPadDialog : public PinDialog {
 private:
	unsigned int timerID;
	int timeoutCounter;
	bool pinpadDialogThreadCompleted;

	static int updateProgressBar(void *object) {
		PinPadDialog *pinPadDialog = static_cast<PinPadDialog*>(object);
		pinPadDialog->updateProgressBarWorker();
		return true;
	}

	void updateProgressBarWorker() {
		timeoutCounter--;
		formElements->progressBar.set_fraction((double) timeoutCounter / COUNTDOWN_STEPS);
		if (timeoutCounter == 0) {
			cancelCountdown();
		}
	}

	void cancelCountdown() {
		if (timerID) {
			g_source_remove(timerID);
		}
		timeoutCounter = 0;
	}

	void startCountdown() {
		cancelCountdown();
		timeoutCounter = COUNTDOWN_STEPS;
		timerID = g_timeout_add_seconds(1, updateProgressBar, this);
	}

 public:

	PinPadDialog() {
		pinpadDialogThreadCompleted = true;
		timerID = 0;
		timeoutCounter = COUNTDOWN_STEPS;
		formElements->setPIN2Label(l10nLabels.get("enter PIN2 pinpad"));
		draw();
	}

	void draw() {
		PinDialog::draw();
		formElements->container.pack_start(formElements->progressBar, false, false, 0);
		dialog->show_all_children();
	}
	
	void hide() {
		cancelCountdown();
		PinDialog::hide();
	}

    std::string getPin() {
		std::thread pinpadDialogThread(&PinPadDialog::run, this);
		pinpadDialogThread.detach();
		startCountdown();
		return "";
	}

	int run() {
		while (timeoutCounter > 0) {
			pinpadDialogThreadCompleted = false;
			dialog->run();
		}
		pinpadDialogThreadCompleted = true;
		return 0;
	}
	
	~PinPadDialog() {
		std::chrono::milliseconds sleepTime(100);
		while (!pinpadDialogThreadCompleted) {
			std::this_thread::sleep_for(sleepTime);
		}
	}
};


#endif	/* PINPADDIALOG_H */

