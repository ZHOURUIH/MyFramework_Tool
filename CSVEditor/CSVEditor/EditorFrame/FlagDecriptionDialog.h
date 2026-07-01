#include "FrameHeader.h"

// 字段标签说明窗口
class FlagDecriptionDialog : public wxDialog
{
public:
	FlagDecriptionDialog(wxWindow* parent);
private:
	static wxString getHelpText();
};