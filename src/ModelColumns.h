#include <gtkmm.h>
#include <string.h>

#ifndef MINICHAN_MODELCOLUMNS
#define MINICHAN_MODELCOLUMNS

using namespace std;
class ModelColumns : public Gtk::TreeModel::ColumnRecord {
	///The model for the board list
	public:

		ModelColumns()
		{ add(name); add(longname);}

		Gtk::TreeModelColumn<string> name;
		Gtk::TreeModelColumn<string> longname;
};

#endif // MINICHAN_MODELCOLUMNS
