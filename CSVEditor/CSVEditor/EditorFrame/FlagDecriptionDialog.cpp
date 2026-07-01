#include "AllHeader.h"

FlagDecriptionDialog::FlagDecriptionDialog(wxWindow* parent):
	wxDialog(parent, wxID_ANY, wxString::FromUTF8("字段标签说明"), wxDefaultPosition, wxSize(760, 520), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
	wxBoxSizer* rootSizer = new wxBoxSizer(wxVERTICAL);

	wxStaticText* titleText = new wxStaticText(this, wxID_ANY, wxString::FromUTF8("字段标签说明"));
	wxFont titleFont = titleText->GetFont();
	titleFont.SetPointSize(titleFont.GetPointSize() + 4);
	titleFont.SetWeight(wxFONTWEIGHT_BOLD);
	titleText->SetFont(titleFont);

	wxScrolledWindow* scrollWindow = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
	scrollWindow->SetScrollRate(0, 20);

	wxBoxSizer* scrollSizer = new wxBoxSizer(wxVERTICAL);

	wxStaticText* contentText = new wxStaticText(scrollWindow, wxID_ANY, getHelpText());
	contentText->Wrap(700);

	scrollSizer->Add(contentText, 0, wxALL | wxEXPAND, 10);
	scrollWindow->SetSizer(scrollSizer);

	rootSizer->Add(titleText, 0, wxALL, 10);
	rootSizer->Add(scrollWindow, 1, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 10);

	SetSizer(rootSizer);
	CentreOnParent();
}

wxString FlagDecriptionDialog::getHelpText()
{
	return (R"(Path:参数支持NotAllowSpace和AllowSpace两种
NotAllowSpace:生成的代码检查路径时不允许路径中携带空格
AllowSpace:生成的代码检查路径时允许路径中携带空格
写法示例 Path:NotAllowSpace

ItemName:参数支持一个字段名,也就是当前表中对应的物品ID的字段
写法示例 ItemName:ItemID
需要注意的是此标签用法仅在拥有Item表时才会正常生效

PropertyName:参数支持一个字段名,也就是当前表中对应的属性ID的字段
写法示例 PropertyName:PropertyType
需要注意的是此标签用法仅在代码中拥有mPropertyName的映射字典定义时才会正常生效

EquipName:参数支持一个字段名,也就是当前表中对应的装备类型ID的字段
写法示例 EquipName:EquipType
需要注意的是此标签用法仅在代码中拥有mEquipName的映射字典定义时才会正常生效)");
}