#ifndef TEMPLATEMANAGER_HPP
#define TEMPLATEMANAGER_HPP

#include <vector>
#include <sstream>
#include <functional>

#include <iostream>

#include "abstract-manager.hpp"
#include "main-window.hpp"
#include "ui_main-window.h"

class MainWindow;

template<class F, class G, class L>
class TemplateManager:
      public AbstractManager
{
protected:
	MainWindow *main_window;
	int count;
	std::vector<F *> vector_forms;
	std::vector<G *> vector_graphics;
	L *lock;
protected:
	TemplateManager (std::string label, MainWindow *main_window):
	   AbstractManager (label),
	   main_window (main_window),
	   count (0),
	   lock (new L ())
	{
	}
public:
	virtual ~TemplateManager () {}
	virtual void setup_components (int width, int height)
	{
		this->main_window->ui->maskFormsTabWidget->clear ();
		this->update_number_components (width, height);
		if (this->main_window->previous_lock) {
			this->main_window->ui->maskLockLayout->removeWidget (this->main_window->previous_lock);
		}
		this->main_window->ui->maskLockLayout->addWidget (this->lock);
		this->main_window->previous_lock = this->lock;
	}
	virtual void update_number_components (int width, int height)
	{
		this->count = this->main_window->ui->numberMasksSpinBox->value ();
		for (int i = this->vector_forms.size (); i < this->main_window->ui->numberMasksSpinBox->value (); i++) {
			F *form = new F (this->main_window->ui->maskFormsTabWidget, width, height);
			G *graphics = new G (this->main_window->pixmap, form, width, height);
			this->vector_forms.push_back (form);
			this->vector_graphics.push_back (graphics);
			this->connect_components (form, graphics);
		}
		// add extra components
		for (int i = this->main_window->ui->maskFormsTabWidget->count (); i < this->main_window->ui->numberMasksSpinBox->value (); i++) {
			std::ostringstream oss;
			oss << "#" << (i + 1);
			this->main_window->ui->maskFormsTabWidget->addTab (vector_forms [i], oss.str ().c_str ());
		}
		// remove excess widgets
		for (int i = this->main_window->ui->numberMasksSpinBox->value (); i < this->main_window->ui->maskFormsTabWidget->count (); i++) {
			this->main_window->ui->maskFormsTabWidget->removeTab (this->main_window->ui->numberMasksSpinBox->value ());
		}
	}
#define LOCK_COMPONENTS(STATE, SPINBOX, PROPERTY, FORM_T, GRAPHICS_T) \
	/* enable the forms if it is unlocked, or if it is locked, only the visible form is enabled */ \
	int index = this->main_window->ui->maskFormsTabWidget->currentIndex ();                        \
	std::cout << "current selected form " << index + 1 << '\n'; \
	std::cout << "state is " << STATE << '\n'; \
	QWidget *widget = this->main_window->ui->maskFormsTabWidget->widget (index);                   \
	for (FORM_T *f: this->vector_forms) {                                                    \
	   bool enabled = STATE == Qt::Unchecked || (STATE == Qt::Checked && f == widget);             \
	   f->ui->SPINBOX->setEnabled (enabled);                                                       \
   }                                                                                              \
	if (STATE == Qt::Unchecked) {                                                                  \
	   /* graphics use their inner radius */                                                       \
	   for (GRAPHICS_T *g: this->vector_graphics) {                                          \
	      g->PROPERTY = 0;                                                                         \
      }                                                                                           \
   }                                                                                              \
	else if (STATE == Qt::Checked) {                                                               \
	   /* graphics use the property of visible form */                                             \
	   for (GRAPHICS_T *g: this->vector_graphics) {                                                \
	      g->PROPERTY = g != this->vector_graphics [index] ? this->vector_graphics [index] : 0;    \
      }                                                                                           \
   }

	virtual void create_mask_files (const std::string &folder, int width, int height, bool use_default_names)
	{
		std::vector<std::string> names;
		unsigned int image_index = 1;
		for (int i = 0; i < this->count; i++) {
			G *g = this->vector_graphics [i];
			F *f = this->vector_forms [i];
			if (!use_default_names)
				names = f->get_mask_names ();
			image_index = g->save_masks (folder, width, height, names, image_index);
		}
	}

	virtual void create_property_files (const std::string &folder)
	{
		unsigned int mask_index = 1;
		for (G *g : this->vector_graphics) {
			g->save_properties (folder, mask_index++);
		}
	}
	void virtual connect_components (F *form, G *graphics) = 0;
};

#endif // TEMPLATEMANAGER_HPP
