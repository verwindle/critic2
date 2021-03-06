/*
  Copyright (c) 2017 Alberto Otero de la Roza
  <aoterodelaroza@gmail.com>, Robin Myhr <x@example.com>, Isaac
  Visintainer <x@example.com>, Richard Greaves <x@example.com>, Ángel
  Martín Pendás <angel@fluor.quimica.uniovi.es> and Víctor Luaña
  <victor@fluor.quimica.uniovi.es>.

  critic2 is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or (at
  your option) any later version.

  critic2 is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "imgui/imgui_dock.h"
#include "imgui/imgui_widgets.h"

#include "dialog.h"
#include "view.h"
#include "settings.h"
#include "keybinding.h"
#include "shapes.h"
#include "message.h"
#include "tree.h"
#include "critic2.h"

using namespace ImGui;

// Declarations for the dialog static functions (defined below)
static void DialogPreferences(bool *p_open);
static void DialogTree(bool *p_open);
static void DialogStructInfo(bool *p_open);

// Dialog variables
bool dlgopen[DLG_LAST] = {false,false};
Dock *dlgdock[DLG_LAST] = {nullptr,nullptr};
Dialog_ dlglastopen = DLG_LAST;

static void (*dlgfun[DLG_LAST])(bool *) = {
  DialogPreferences,
  DialogTree,
  DialogStructInfo,
};
static const float dlgsplitweight[][3] = {
  {2.f,2.f},
  {1.f,4.f},
  {2.f,2.f},
};

void OpenDialog(Dialog_ dialog){
  if (!dlgopen[dialog])
    dlgopen[dialog] = true;
  else if (dlgdock[dialog] && dlgdock[dialog]->window){
    if (dlgdock[dialog]->status == Dock::Status_Docked){
      dlgdock[dialog]->parent->currenttab = dlgdock[dialog];
      dlgdock[dialog]->parent->focusContainer();      
    } else {
      FocusWindow(dlgdock[dialog]->window);
    }
  }
  dlglastopen = dialog;
}
void ToggleDialog(Dialog_ dialog){
  if (!dlgopen[dialog]){
    dlgopen[dialog] = true;
    dlglastopen = dialog;
  } else {
    dlgopen[dialog] = false;
    if (dlgdock[dialog])
      dlgdock[dialog]->closeDock();
    if (dlglastopen == dialog)
      dlglastopen = DLG_LAST;
  }
}

void DialogDispatch(){
  ImGuiContext *g = GetCurrentContext();

  // Detect the close-all and close-one
  if (IsBindEvent(BIND_CLOSE_LAST_DIALOG,false) && g->OpenPopupStack.empty())
    CloseLastDialog();
  if (IsBindEvent(BIND_CLOSE_ALL_DIALOGS,false))
    CloseAllDialogs();

  // Process all the flags
  for (int i = 0; i < DLG_LAST; i++){
    if (dlgopen[i])
      dlgfun[i](&(dlgopen[i]));
  }
}

void CloseAllDialogs(){
  for (int i = 0; i < DLG_LAST; i++){
    dlgopen[i] = false;
    if (dlgdock[i])
      dlgdock[i]->closeDock();
  }
  dlglastopen = DLG_LAST;
}

void CloseLastDialog(){
  if (dlglastopen != DLG_LAST){
    dlgopen[dlglastopen] = false;
    if (dlgdock[dlglastopen])
      dlgdock[dlglastopen]->closeDock();
  }
  dlglastopen = DLG_LAST;
}

// Static functions for all dialogs known to the dispatcher //

// Preferences dialog //
static void DialogPreferences(bool *p_open){
  ImGuiContext *g = GetCurrentContext();
  ImGuiIO& io = GetIO();
  ImGuiStyle& style = GetStyle();
  float itemwidth = 4.f * g->FontSize;
  static ImGuiTextFilter filter;
  const float wleft = 150.f;
  const float wright = 400.f;

  if (*p_open){
    static bool first = true;
    SetNextWindowSize(ImVec2(0.5f*io.DisplaySize.x,0.5f*io.DisplaySize.y), ImGuiSetCond_FirstUseEver);

    if (BeginDock("Preferences",p_open)){
      bool updateviews = false;

      // Filter box
      AlignTextToFramePadding();
      Text("Filter");
      SameLine();
      filter.Draw("",-1.f);

      // Selectable categories
      const int ncat = 5;
      char *catname[ncat] = {
	"Interface","Views","Key bindings","Colors","Fonts"
      };

      // Left panel
      static int catid = 0;
      BeginChild("leftpanel", ImVec2(wleft, 0), true);
      for (int i=0; i < ncat; i++){
	if (Selectable(catname[i], catid == i))
	  catid = i;
      }
      EndChild();
      SameLine();

      // Right panel
      BeginGroup();
      BeginChild("rightpanel", ImVec2(0,-GetItemsLineHeightWithSpacing()-g->Style.ItemSpacing.y));
      bool setexpcol = false, expcol = false;
      AlignTextToFramePadding();
      Text(catname[catid]); 
      if (catid == 0 || catid == 1){
	SameLine();
	VerticalSeparator();
	SameLine();
	if (Button("Expand")){
	  setexpcol = true; 
	  expcol = true;
	}
	SameLine();
	if (Button("Collapse")){
	  setexpcol = true; 
	  expcol = false;
	}
      }
      Separator();
      PushItemWidth(itemwidth);
      if (catid == 0){
	// Interface
	if (setexpcol) SetNextTreeNodeOpen(expcol);
	if (TreeNode("High DPI")){
	  float oscale = ImGuiStyleUI.UIScaleFactor;
	  if (filter.PassFilter("Global UI scale factor"))
	    updateviews |= DragFloat("Global UI scale factor", &ImGuiStyleUI.UIScaleFactor, 0.01f, 0.25f, 5.0f, "%.1f", 1.0f);
	  if (ImGuiStyleUI.UIScaleFactor != oscale){
	    ScaleUI(ImGuiStyleUI.UIScaleFactor / oscale);
	    oscale = ImGuiStyleUI.UIScaleFactor;
	  }
	  TreePop();
	}
	if (setexpcol) SetNextTreeNodeOpen(expcol);
	if (TreeNode("Tooltips")){
	  if (filter.PassFilter("Enable tooltips"))
	    Checkbox("Enable tooltips", &ImGuiStyleUI.TooltipEnabled);
	  if (filter.PassFilter("Tooltip delay (s)"))
	    DragFloat("Tooltip delay (s)", &ImGuiStyleUI.TooltipDelay, 0.1f, 0.0f, FLT_MAX, "%.1f", 1.0f);
	  if (filter.PassFilter("Tooltip maximum width (pixel)"))
	    DragFloat("Tooltip maximum width (pixel)", &ImGuiStyleUI.TooltipMaxwidth, 5.0f, 0.0f, FLT_MAX, "%.1f", 1.0f);
	  TreePop();
	}
	if (setexpcol) SetNextTreeNodeOpen(expcol);
	if (TreeNode("Drop targets")){
	  if (filter.PassFilter("Drop target looseness"))
	    SliderFloat("Drop target looseness", &ImGuiStyleWidgets.DropTargetLooseness, 0.0f, 48.0f, "%.0f");
	  if (filter.PassFilter("Drop target minimum size"))
	    SliderFloat("Drop target minimum size", &ImGuiStyleWidgets.DropTargetMinsizeEdge, 0.0f, 150.0f, "%.0f");
	  if (filter.PassFilter("Drop target maximum size"))
	    SliderFloat("Drop target maximum size", &ImGuiStyleWidgets.DropTargetMaxsizeEdge, 0.0f, 150.0f, "%.0f");
	  if (filter.PassFilter("Drop target edge fraction"))
	    SliderFloat("Drop target edge fraction", &ImGuiStyleWidgets.DropTargetEdgeFraction, 0.0f, 0.5f, "%.2f");
	  if (filter.PassFilter("Drop target body fraction"))
	    SliderFloat("Drop target body fraction", &ImGuiStyleWidgets.DropTargetFullFraction, 0.0f, 0.5f, "%.2f");
	  TreePop();
	}
	if (setexpcol) SetNextTreeNodeOpen(expcol);
	if (TreeNode("Messages")){
	  if (filter.PassFilter("Message width"))
	    DragFloat("Message width", &ImGuiStyleUI.MessageWidth, 1.f, 1.0, FLT_MAX, "%.0f", 1.0f); 
	  if (filter.PassFilter("Message expiration time (s)"))
	    DragFloat("Message expiration time (s)", &ImGuiStyleUI.MessageExpire, 0.2f, 0.0, 60.0, "%.1f", 1.0f); 
	  TreePop();
	}
	if (setexpcol) SetNextTreeNodeOpen(expcol);
	if (TreeNode("Borders")){
	  // Interface -> Settings
	  bool border = (style.WindowBorderSize > 0.0f);
	  if (filter.PassFilter("Window borders") && Checkbox("Window borders", &border))
	    style.WindowBorderSize = border ? 1.0f : 0.0f;
	  border = (style.FrameBorderSize > 0.0f);
	  if (filter.PassFilter("Frame borders") && Checkbox("Frame borders", &border))
	    style.FrameBorderSize = border ? 1.0f : 0.0f;
	  border = (ImGuiStyleWidgets.TabBorderSize > 0.0f);
	  if (filter.PassFilter("Tab borders") && Checkbox("Tab borders", &border))
	    ImGuiStyleWidgets.TabBorderSize = border ? 1.0f : 0.0f;
	  border = (style.PopupBorderSize > 0.0f);
	  if (filter.PassFilter("Popup borders") && Checkbox("Popup borders", &border))
	    style.PopupBorderSize = border ? 1.0f : 0.0f;
	  border = (style.ChildBorderSize > 0.0f);
	  TreePop();
	}
	if (setexpcol) SetNextTreeNodeOpen(expcol);
	if (TreeNode("Element size and positioning")){
	  PushItemWidth(2 * itemwidth + 1.f * g->Style.ItemInnerSpacing.x);
	  if (filter.PassFilter("Window padding"))
	    SliderFloat2("Window padding", (float*)&style.WindowPadding, 0.0f, 20.0f, "%.0f");
	  if (filter.PassFilter("Frame padding"))
	    SliderFloat2("Frame padding", (float*)&style.FramePadding, 0.0f, 20.0f, "%.0f");
	  if (filter.PassFilter("Item spacing"))
	    SliderFloat2("Item spacing", (float*)&style.ItemSpacing, 0.0f, 20.0f, "%.0f");
	  if (filter.PassFilter("Item inner spacing"))
	    SliderFloat2("Item inner spacing", (float*)&style.ItemInnerSpacing, 0.0f, 20.0f, "%.0f");
	  if (filter.PassFilter("Touch extra padding"))
	    SliderFloat2("Touch extra padding", (float*)&style.TouchExtraPadding, 0.0f, 10.0f, "%.0f");
	  PopItemWidth();
	  if (filter.PassFilter("Indent spacing"))
	    SliderFloat("Indent spacing", &style.IndentSpacing, 0.0f, 30.0f, "%.0f");
	  if (filter.PassFilter("Scroll bar width"))
	    SliderFloat("Scroll bar width", &style.ScrollbarSize, 1.0f, 20.0f, "%.0f");
	  if (filter.PassFilter("Sliding bar width"))
	    SliderFloat("Sliding bar width", &ImGuiStyleWidgets.SlidingBarWidth, 1.0f, 20.0f, "%.0f");
	  if (filter.PassFilter("Grab size"))
	    SliderFloat("Grab size", &style.GrabMinSize, 1.0f, 20.0f, "%.0f");
	  if (filter.PassFilter("Tab height"))
	    SliderFloat("Tab height", &ImGuiStyleWidgets.TabHeight, 6.0f, 42.0f, "%.0f");
	  if (filter.PassFilter("Tab maximum width"))
	    SliderFloat("Tab maximum width", &ImGuiStyleWidgets.TabMaxWidth, 25.0f, 200.0f, "%.0f");
	  TreePop();
	}
	if (setexpcol) SetNextTreeNodeOpen(expcol);
	if (TreeNode("Rounding")){
	  if (filter.PassFilter("Window rounding"))
	    SliderFloat("Window rounding", &style.WindowRounding, 0.0f, 14.0f, "%.0f");
	  if (filter.PassFilter("Child window rounding"))
	    SliderFloat("Child window rounding", &style.ChildRounding, 0.0f, 16.0f, "%.0f");
	  if (filter.PassFilter("Frame rounding") && SliderFloat("Frame rounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f"))
	    style.GrabRounding = style.FrameRounding;
	  if (filter.PassFilter("Scroll bar rounding"))
	    SliderFloat("Scroll bar rounding", &style.ScrollbarRounding, 0.0f, 12.0f, "%.0f");
	  if (filter.PassFilter("Grab rounding"))
	    SliderFloat("Grab rounding", &style.GrabRounding, 0.0f, 12.0f, "%.0f");
	  if (filter.PassFilter("Tab rounding"))
	    SliderFloat("Tab rounding", &ImGuiStyleWidgets.TabRounding, 0.0f, 14.0f, "%.0f");
	  if (filter.PassFilter("Popup rounding"))
	    SliderFloat("Popup rounding", &style.PopupRounding, 0.0f, 16.0f, "%.0f");
	  TreePop();
	}
	if (setexpcol) SetNextTreeNodeOpen(expcol);
	if (TreeNode("Alignment")){
	  PushItemWidth(2 * itemwidth + 1.f * g->Style.ItemInnerSpacing.x);
	  if (filter.PassFilter("Window title alignment"))
	    SliderFloat2("Window title alignment", (float*)&style.WindowTitleAlign, 0.0f, 1.0f, "%.2f");
	  if (filter.PassFilter("Button text alignment"))
	    SliderFloat2("Button text alignment", (float*)&style.ButtonTextAlign, 0.0f, 1.0f, "%.2f");
	  PopItemWidth();
	  TreePop();
	}
      } else if (catid == 1){
	// Views
	if (setexpcol) SetNextTreeNodeOpen(expcol);
	if (TreeNode("Lighting")){
          bool changed = false;
	  // Views -> Lighting
	  PushItemWidth(3 * itemwidth + 2.f * g->Style.ItemInnerSpacing.x);
	  if (filter.PassFilter("Light position"))
	    if (changed = DragFloat3("Light position", &(view_lightpos[0]), 0.5f, -FLT_MAX, FLT_MAX, "%.1f", 1.0f))
              SetDefaultAllViews(View::V_lightpos);
          updateviews |= changed;

	  if (filter.PassFilter("Light color"))
	    if (changed = ColorEdit3("Light color", &(view_lightcolor[0]), coloreditflags))
              SetDefaultAllViews(View::V_lightcolor);
          updateviews |= changed;
          PopItemWidth();

	  if (filter.PassFilter("Ambient light intensity"))
	    if (changed = DragFloat("Ambient light intensity", &view_ambient, 0.002f, 0.0f, 1.0f, "%.3f", 1.0f))
              SetDefaultAllViews(View::V_ambient);
          updateviews |= changed;

	  if (filter.PassFilter("Diffuse light intensity"))
	    if (changed = DragFloat("Diffuse light intensity", &view_diffuse, 0.002f, 0.0f, 1.0f, "%.3f", 1.0f))
              SetDefaultAllViews(View::V_diffuse);
          updateviews |= changed;

	  if (filter.PassFilter("Specular light intensity"))
	    if (changed = DragFloat("Specular light intensity", &view_specular, 0.002f, 0.0f, 1.0f, "%.3f", 1.0f))
              SetDefaultAllViews(View::V_specular);
	  updateviews |= changed;

          if (filter.PassFilter("Light shininess"))
            if (changed = DragInt("Light shininess", &view_shininess, 1.0f, 0.0f, 256.f, "%.0f"))
              SetDefaultAllViews(View::V_shininess);
          updateviews |= changed;
	  TreePop();
	}
	if (setexpcol) SetNextTreeNodeOpen(expcol);
	if (TreeNode("Mouse sensitivity")){
	  if (filter.PassFilter("Rotation mouse sensitivity"))
	    DragFloat("Rotation mouse sensitivity", &view_mousesens_rot, 0.02f, 0.0f, 10.f, "%.2f", 1.0f); 
	  if (filter.PassFilter("Zoom mouse sensitivity"))
	    DragFloat("Zoom mouse sensitivity", &view_mousesens_zoom, 0.02f, 0.0f, 10.f, "%.2f", 1.0f); 
	  TreePop();
	}
	if (setexpcol) SetNextTreeNodeOpen(expcol);
	if (TreeNode("Scene settings")){
	  bool changed = false;
	  if (filter.PassFilter("Wireframe rendering"))
	    if (changed = Checkbox("Wireframe rendering", &view_wireframe))
              SetDefaultAllViews(View::V_wireframe);
          updateviews |= changed;

	  if (filter.PassFilter("Orthgonal projection"))
	    if (changed = Checkbox("Orthgonal projection", &view_orthogonal))
              SetDefaultAllViews(View::V_orthogonal);
          updateviews |= changed;

	  if (filter.PassFilter("Field of view (degrees)"))
	    if (changed = DragFloat("Field of view (degrees)", &view_fov, 2.5f, 0.0f, 180.0f, "%.1f", 1.0f))
              SetDefaultAllViews(View::V_fov);
          updateviews |= changed;

	  if (filter.PassFilter("Reset distance (scene radius)"))
	    if (changed = DragFloat("Reset distance (scene radius)", &view_resetdistance, 0.02f, 0.0f, 10.f, "%.2f", 1.0f))
              SetDefaultAllViews(View::V_resetdistance);
          updateviews |= changed;

	  if (filter.PassFilter("Background color"))
	    if (changed = ColorEdit4("Background color", view_bgrgb, coloreditflags))
              SetDefaultAllViews(View::V_bgrgb);
          updateviews |= changed;

	  if (filter.PassFilter("Show atoms"))
	    if (changed = Checkbox("Show atoms", &view_show_atoms))
              SetDefaultAllViews(View::V_show_atoms);
          updateviews |= changed;

	  if (filter.PassFilter("Atom resolution"))
	    if (changed = SliderInt("Atom resolution", &view_isphres, 0, nmaxsph-1))
              SetDefaultAllViews(View::V_isphres);
          updateviews |= changed;

	  if (filter.PassFilter("Show bonds"))
	    if (changed = Checkbox("Show bonds", &view_show_bonds))
              SetDefaultAllViews(View::V_show_bonds);
          updateviews |= changed;

	  if (filter.PassFilter("Bond resolution"))
	    if (changed = SliderInt("Bond resolution", &view_icylres, 0, nmaxcyl-1))
              SetDefaultAllViews(View::V_icylres);
          updateviews |= changed;

	  if (filter.PassFilter("Show labels"))
	    if (changed = Checkbox("Show labels", &view_show_labels))
              SetDefaultAllViews(View::V_show_labels);
          updateviews |= changed;

	  if (filter.PassFilter("Label text")){
            PushItemWidth(3.0f * itemwidth);
	    if (changed = Combo("Label text", &view_format_labels, "Number\0Number (sym-only)\0Name\0Symbol\0Fragment\0"))
              SetDefaultAllViews(View::V_format_labels);
            PopItemWidth();
          }
          updateviews |= changed;

	  if (filter.PassFilter("Label shows lattice vector"))
            if (changed = Checkbox("Label shows lattice vector", &view_lat_labels))
              SetDefaultAllViews(View::V_lat_labels);
          updateviews |= changed;

	  if (filter.PassFilter("Label size"))
            if (changed = DragFloat("Label size", &view_scale_labels, 0.01f, 0.0f, 5.f, "%.2f", 1.0f))
              SetDefaultAllViews(View::V_scale_labels);
          updateviews |= changed;

	  if (filter.PassFilter("Label color"))
	    if (changed = ColorEdit3("label color", view_rgb_labels, coloreditflags))
              SetDefaultAllViews(View::V_rgb_labels);
          updateviews |= changed;

	  TreePop();
	}
      } else if (catid == 2){
	static int getbind = -1;
	TextDisabled("(Right-click on the button to toggle double-click)");

	// Key bindings
	BeginChild("keybindselector",ImVec2(wright,0.f),false);
	Columns(2,"keybindingcolumns",false);
	for (int i = 0; i < BIND_MAX; i++){
	  if (!filter.PassFilter(BindNames[i]))
	    continue;
	  string result = BindKeyName(i);
	  if (result.length() == 0)
	    result = "<not bound>";
	  Text(BindNames[i]); 
	  NextColumn();

	  PushID(i);
	  if (Button(result.c_str())){
	    getbind = i;
	  }
	  PopID();

	  if (IsMouseClicked(1) && IsItemHovered()){
	    int newkey;
	    switch (keybind[i]){
	    case GLFW_MOUSE_LEFT: 
	      newkey = GLFW_MOUSE_LEFT_DOUBLE; break;
	    case GLFW_MOUSE_LEFT_DOUBLE: 
	      newkey = GLFW_MOUSE_LEFT; break;
	    case GLFW_MOUSE_RIGHT: 
	      newkey = GLFW_MOUSE_RIGHT_DOUBLE; break;
	    case GLFW_MOUSE_RIGHT_DOUBLE: 
	      newkey = GLFW_MOUSE_RIGHT; break;
	    case GLFW_MOUSE_MIDDLE: 
	      newkey = GLFW_MOUSE_MIDDLE_DOUBLE; break;
	    case GLFW_MOUSE_MIDDLE_DOUBLE: 
	      newkey = GLFW_MOUSE_MIDDLE; break;
	    case GLFW_MOUSE_BUTTON3: 
	      newkey = GLFW_MOUSE_BUTTON3_DOUBLE; break;
	    case GLFW_MOUSE_BUTTON3_DOUBLE: 
	      newkey = GLFW_MOUSE_BUTTON3; break;
	    case GLFW_MOUSE_BUTTON4: 
	      newkey = GLFW_MOUSE_BUTTON4_DOUBLE; break;
	    case GLFW_MOUSE_BUTTON4_DOUBLE: 
	      newkey = GLFW_MOUSE_BUTTON4; break;
	    default:
	      newkey = NOKEY;
	    }
	    if (newkey)
	      SetBind(i,newkey,modbind[i]);
	  }
	  NextColumn();
	}
	Columns(1);
	EndChild();

	// popup to read a key
	if (getbind != -1){
	  SetBindEventLevel(1);
	  OpenPopup("Choosekey");
	  if (BeginPopupModal("Choosekey", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar |
			      ImGuiWindowFlags_NoMove)){
	    Text("Please press a key or mouse button.");
	    if (SetBindFromUserInput(getbind)){
	      getbind = -1;
	      SetBindEventLevel();
	      CloseCurrentPopup(); 
	    }
	    EndPopup();
	  }
	}

      } else if (catid == 3){
	// Colors
	static int style_idx = 0;
	int ostyle_idx = style_idx;
	PushItemWidth(3 * itemwidth + 2.f * g->Style.ItemInnerSpacing.x);
	if (filter.PassFilter("Color theme")){
	  if (Combo("Color theme", &style_idx, "Mutant Orange\0Classic\0Dark\0Light\0",4)){
	    if (style_idx != ostyle_idx){
	      switch (style_idx){
	      case 0: UIStyleColorsMutantOrange(); break;
	      case 1: UIStyleColorsClassic(); break;
	      case 2: UIStyleColorsDark(); break;
	      case 3: UIStyleColorsLight(); break;
	      }
	    }
	  }
	}
	PopItemWidth();
	Separator();

	// Interface -> Colors
	if (filter.PassFilter("Text"))
	  ColorEdit4("Text", (float*)&(style.Colors[ImGuiCol_Text]), coloreditflags);
	if (filter.PassFilter("Text (Disabled)"))
	  ColorEdit4("Text (Disabled)", (float*)&style.Colors[ImGuiCol_TextDisabled], coloreditflags);
	if (filter.PassFilter("Text (Selected background)"))
	  ColorEdit4("Text (Selected background)", (float*)&style.Colors[ImGuiCol_TextSelectedBg], coloreditflags);
	if (filter.PassFilter("Backdrop"))
	  ColorEdit4("Backdrop", (float*)&ImGuiStyleUI.Colors[ImGuiColUI_BackDrop], coloreditflags);
	if (filter.PassFilter("Window background"))
	  ColorEdit4("Window background", (float*)&style.Colors[ImGuiCol_WindowBg], coloreditflags);
	if (filter.PassFilter("Child window background"))
	  ColorEdit4("Child window background", (float*)&style.Colors[ImGuiCol_ChildBg], coloreditflags);
	if (filter.PassFilter("Popup background"))
	  ColorEdit4("Popup background", (float*)&style.Colors[ImGuiCol_PopupBg], coloreditflags);
	if (filter.PassFilter("Item frame"))
	  ColorEdit4("Item frame", (float*)&style.Colors[ImGuiCol_FrameBg], coloreditflags);
	if (filter.PassFilter("Item frame (Hovered)"))
	  ColorEdit4("Item frame (Hovered)", (float*)&style.Colors[ImGuiCol_FrameBgHovered], coloreditflags);
	if (filter.PassFilter("Item frame (Active)"))
	  ColorEdit4("Item frame (Active)", (float*)&style.Colors[ImGuiCol_FrameBgActive], coloreditflags);
	if (filter.PassFilter("Item header"))
	  ColorEdit4("Item header", (float*)&style.Colors[ImGuiCol_Header], coloreditflags);
	if (filter.PassFilter("Item header (Hovered)"))
	  ColorEdit4("Item header (Hovered)", (float*)&style.Colors[ImGuiCol_HeaderHovered], coloreditflags);
	if (filter.PassFilter("Item header (Active)"))
	  ColorEdit4("Item header (Active)", (float*)&style.Colors[ImGuiCol_HeaderActive], coloreditflags);
	if (filter.PassFilter("Button"))
	  ColorEdit4("Button", (float*)&style.Colors[ImGuiCol_Button], coloreditflags);
	if (filter.PassFilter("Button (Hovered)"))
	  ColorEdit4("Button (Hovered)", (float*)&style.Colors[ImGuiCol_ButtonHovered], coloreditflags);
	if (filter.PassFilter("Button (Pressed)"))
	  ColorEdit4("Button (Pressed)", (float*)&style.Colors[ImGuiCol_ButtonActive], coloreditflags);
	if (filter.PassFilter("Close button"))
	  ColorEdit4("Close button", (float*)&style.Colors[ImGuiCol_CloseButton], coloreditflags);
	if (filter.PassFilter("Close button (Hovered)"))
	  ColorEdit4("Close button (Hovered)", (float*)&style.Colors[ImGuiCol_CloseButtonHovered], coloreditflags);
	if (filter.PassFilter("Close button (Pressed)"))
	  ColorEdit4("Close button (Pressed)", (float*)&style.Colors[ImGuiCol_CloseButtonActive], coloreditflags);
	if (filter.PassFilter("Window title"))
	  ColorEdit4("Window title", (float*)&style.Colors[ImGuiCol_TitleBg], coloreditflags);
	if (filter.PassFilter("Window title (Focused)"))
	  ColorEdit4("Window title (Focused)", (float*)&style.Colors[ImGuiCol_TitleBgActive], coloreditflags);
	if (filter.PassFilter("Window title (Collapsed)"))
	  ColorEdit4("Window title (Collapsed)", (float*)&style.Colors[ImGuiCol_TitleBgCollapsed], coloreditflags);
	if (filter.PassFilter("Resize grip"))
	  ColorEdit4("Resize grip", (float*)&style.Colors[ImGuiCol_ResizeGrip], coloreditflags);
	if (filter.PassFilter("Resize grip (Hovered)"))
	  ColorEdit4("Resize grip (Hovered)", (float*)&style.Colors[ImGuiCol_ResizeGripHovered], coloreditflags);
	if (filter.PassFilter("Resize grip (Grabbed)"))
	  ColorEdit4("Resize grip (Grabbed)", (float*)&style.Colors[ImGuiCol_ResizeGripActive], coloreditflags);
	if (filter.PassFilter("Menu Bar"))
	  ColorEdit4("Menu Bar", (float*)&style.Colors[ImGuiCol_MenuBarBg], coloreditflags);
	if (filter.PassFilter("Window border"))
	  ColorEdit4("Window border", (float*)&style.Colors[ImGuiCol_Border], coloreditflags);
	if (filter.PassFilter("Window border shadow"))
	  ColorEdit4("Window border shadow", (float*)&style.Colors[ImGuiCol_BorderShadow], coloreditflags);
	if (filter.PassFilter("Scroll bar background"))
	  ColorEdit4("Scroll bar background", (float*)&style.Colors[ImGuiCol_ScrollbarBg], coloreditflags);
	if (filter.PassFilter("Scroll bar grab item"))
	  ColorEdit4("Scroll bar grab item", (float*)&style.Colors[ImGuiCol_ScrollbarGrab], coloreditflags);
	if (filter.PassFilter("Scroll bar grab item (Hovered)"))
	  ColorEdit4("Scroll bar grab item (Hovered)", (float*)&style.Colors[ImGuiCol_ScrollbarGrabHovered], coloreditflags);
	if (filter.PassFilter("Scroll bar grab item (Grabbed)"))
	  ColorEdit4("Scroll bar grab item (Grabbed)", (float*)&style.Colors[ImGuiCol_ScrollbarGrabActive], coloreditflags);
	if (filter.PassFilter("Check mark"))
	  ColorEdit4("Check mark", (float*)&style.Colors[ImGuiCol_CheckMark], coloreditflags);
	if (filter.PassFilter("Slider grab item"))
	  ColorEdit4("Slider grab item", (float*)&style.Colors[ImGuiCol_SliderGrab], coloreditflags);
	if (filter.PassFilter("Slider grab item (Grabbed)"))
	  ColorEdit4("Slider grab item (Grabbed)", (float*)&style.Colors[ImGuiCol_SliderGrabActive], coloreditflags);
	if (filter.PassFilter("Separator"))
	  ColorEdit4("Separator", (float*)&style.Colors[ImGuiCol_Separator], coloreditflags);
	if (filter.PassFilter("Separator (Hovered)"))
	  ColorEdit4("Separator (Hovered)", (float*)&style.Colors[ImGuiCol_SeparatorHovered], coloreditflags);
	if (filter.PassFilter("Separator (Active)"))
	  ColorEdit4("Separator (Active)", (float*)&style.Colors[ImGuiCol_SeparatorActive], coloreditflags);
	if (filter.PassFilter("Plot lines"))
	  ColorEdit4("Plot lines", (float*)&style.Colors[ImGuiCol_PlotLines], coloreditflags);
	if (filter.PassFilter("Plot lines (Hovered)"))
	  ColorEdit4("Plot lines (Hovered)", (float*)&style.Colors[ImGuiCol_PlotLinesHovered], coloreditflags);
	if (filter.PassFilter("Histogram"))
	  ColorEdit4("Histogram", (float*)&style.Colors[ImGuiCol_PlotHistogram], coloreditflags);
	if (filter.PassFilter("Histogram (Hovered)"))
	  ColorEdit4("Histogram (Hovered)", (float*)&style.Colors[ImGuiCol_PlotHistogramHovered], coloreditflags);
	if (filter.PassFilter("Modal window darkening"))
	  ColorEdit4("Modal window darkening", (float*)&style.Colors[ImGuiCol_ModalWindowDarkening], coloreditflags);
	if (filter.PassFilter("Sliding bar"))
	  ColorEdit4("Sliding bar", (float*)&(ImGuiStyleWidgets.Colors[ImGuiColWidgets_Slidingbar]), coloreditflags);
	if (filter.PassFilter("Sliding bar (Hovered)"))
	  ColorEdit4("Sliding bar (Hovered)", (float*)&(ImGuiStyleWidgets.Colors[ImGuiColWidgets_SlidingbarHovered]), coloreditflags);
	if (filter.PassFilter("Sliding bar (Grabbed)"))
	  ColorEdit4("Sliding bar (Grabbed)", (float*)&(ImGuiStyleWidgets.Colors[ImGuiColWidgets_SlidingbarActive]), coloreditflags);
	if (filter.PassFilter("Tab"))
	  ColorEdit4("Tab", (float*)&(ImGuiStyleWidgets.Colors[ImGuiColWidgets_Tab]), coloreditflags);
	if (filter.PassFilter("Tab (Hovered)"))
	  ColorEdit4("Tab (Hovered)", (float*)&(ImGuiStyleWidgets.Colors[ImGuiColWidgets_TabHovered]), coloreditflags);
	if (filter.PassFilter("Tab (Pressed)"))
	  ColorEdit4("Tab (Pressed)", (float*)&(ImGuiStyleWidgets.Colors[ImGuiColWidgets_TabPressed]), coloreditflags);
	if (filter.PassFilter("Tab (Active)"))
	  ColorEdit4("Tab (Active)", (float*)&(ImGuiStyleWidgets.Colors[ImGuiColWidgets_TabActive]), coloreditflags);
	if (filter.PassFilter("Tab close button foreground"))
	  ColorEdit4("Tab close button foreground", (float*)&(ImGuiStyleWidgets.Colors[ImGuiColWidgets_TabXFg]), coloreditflags);
	if (filter.PassFilter("Tab close button foreground (Hovered)"))
	  ColorEdit4("Tab close button foreground (Hovered)", (float*)&(ImGuiStyleWidgets.Colors[ImGuiColWidgets_TabXFgHovered]), coloreditflags);
	if (filter.PassFilter("Tab close button foreground (Active)"))
	  ColorEdit4("Tab close button foreground (Active)", (float*)&(ImGuiStyleWidgets.Colors[ImGuiColWidgets_TabXFgActive]), coloreditflags);
	if (filter.PassFilter("Tab close button background"))
	  ColorEdit4("Tab close button background", (float*)&(ImGuiStyleWidgets.Colors[ImGuiColWidgets_TabXBg]), coloreditflags);
	if (filter.PassFilter("Tab close button background (Hovered)"))
	  ColorEdit4("Tab close button background (Hovered)", (float*)&(ImGuiStyleWidgets.Colors[ImGuiColWidgets_TabXBgHovered]), coloreditflags);
	if (filter.PassFilter("Tab close button background (Active)"))
	  ColorEdit4("Tab close button background (Active)", (float*)&(ImGuiStyleWidgets.Colors[ImGuiColWidgets_TabXBgActive]), coloreditflags);
	if (filter.PassFilter("Tab border"))
	  ColorEdit4("Tab border", (float*)&(ImGuiStyleWidgets.Colors[ImGuiColWidgets_TabBorder]), coloreditflags);
	if (filter.PassFilter("Drop target"))
	  ColorEdit4("Drop target", (float*)&(ImGuiStyleWidgets.Colors[ImGuiColWidgets_DropTarget]), coloreditflags);
	if (filter.PassFilter("Drop target (Active)"))
	  ColorEdit4("Drop target (Active)", (float*)&(ImGuiStyleWidgets.Colors[ImGuiColWidgets_DropTargetActive]), coloreditflags);
	if (filter.PassFilter("View icon"))
	  ColorEdit4("View icon", (float*)&(ImGuiStyleUI.Colors[ImGuiColUI_ViewIcon]), coloreditflags);
	if (filter.PassFilter("View icon (Hovered)"))
	  ColorEdit4("View icon (Hovered)", (float*)&(ImGuiStyleUI.Colors[ImGuiColUI_ViewIconHovered]), coloreditflags);
	if (filter.PassFilter("View icon (Grabbed)"))
	  ColorEdit4("View icon (Grabbed)", (float*)&(ImGuiStyleUI.Colors[ImGuiColUI_ViewIconActive]), coloreditflags);
	if (filter.PassFilter("View icon (Inactive)"))
	  ColorEdit4("View icon (Inactive)", (float*)&(ImGuiStyleUI.Colors[ImGuiColUI_ViewIconInactive]), coloreditflags);
	if (filter.PassFilter("Message (Info)"))
	  ColorEdit4("Message (Info)", (float*)&(ImGuiStyleUI.Colors[ImGuiColUI_MessageInfo]), coloreditflags);
	if (filter.PassFilter("Message (Warning)"))
	  ColorEdit4("Message (Warning)", (float*)&(ImGuiStyleUI.Colors[ImGuiColUI_MessageWarning]), coloreditflags);
	if (filter.PassFilter("Message (Error)"))
	  ColorEdit4("Message (Error)", (float*)&(ImGuiStyleUI.Colors[ImGuiColUI_MessageError]), coloreditflags);
      } else if (catid == 4){
	// Fonts
	PushItemWidth(1.5*itemwidth); 
	float size = ImGuiStyleUI.FontSize;
	DragFloat("Font size (pixel)", &size, 0.1f, 9.0f, fontsizebake, "%.1f");
	if (size != ImGuiStyleUI.FontSize)
	  SetUIFont(-1,size,-1.f);

	size = ImGuiStyleUI.FontSizeIcon;
	DragFloat("View icon size (pixel)", &size, 0.1f, 9.0f, fontsizebake, "%.1f");
	PopItemWidth(); 
	if (size != ImGuiStyleUI.FontSizeIcon)
	  SetUIFont(-1,-1.f,size);

	BeginChild("fontselector",ImVec2(0.f,0.f),true);
	for (int i = 1; i < io.Fonts->Fonts.Size; i++){ // first font is always the icons
	  ImFont* font = io.Fonts->Fonts[i];
	  PushFont(font);
	  if (Selectable(font->ConfigData?font->ConfigData[0].Name:"",ImGuiStyleUI.FontSelected == i))
	    SetUIFont(i,-1.f,-1.f);
	  PopFont();
	}
	EndChild();
      }
      PopItemWidth();
      EndChild();
      Separator();

      // Line at the bottom
      BeginChild("buttons");
      if (Button("Reset"))
	OpenPopup("resetpopup");
      if (BeginPopup("resetpopup")){
	if (Selectable("Reset to config file")){
	  if (!ReadConfigurationFile(conffile)){
	    NewMessage(Message_Error,"Could not read configuration file.");
	  } else {
	    string message = "Configuration loaded from file:\n" + conffile;
	    NewMessage(Message_Info,message.c_str());
	  }
	}
	if (Selectable("Reset to defaults")){
	  DefaultSettings();
	  SetDefaultAllViews();
	  SetDefaultKeyBindings();
	}
	EndPopup();
      }

      SameLine();
      if (Button("Save"))
	if (!WriteConfigurationFile(conffile))
	  NewMessage(Message_Error,"Could not write configuration file.");
	else{
	  string message = "Configuration saved to file:\n" + conffile;
	  NewMessage(Message_Info,message.c_str());
	}
      SameLine();
      if (Button("OK")){
	*p_open = false;
      }
      EndChild();
      EndGroup();

      if (updateviews)
        ForceUpdateAllViews();
    } // BeginDock()

    dlgdock[DLG_Preferences] = GetCurrentDock();
    if (first){
      dlgdock[DLG_Preferences]->setDetachedDockSize(0.5f*io.DisplaySize.x,0.5f*io.DisplaySize.y);
      dlgdock[DLG_Preferences]->setSplitWeight(dlgsplitweight[DLG_Preferences][0],dlgsplitweight[DLG_Preferences][1]);
      first = false;
    }
    EndDock();
  } // *p_open
  if (!(*p_open))
    filter.Clear();
}

static void DialogTree(bool *p_open){
  ImGuiIO& io = GetIO();
  if (*p_open){
    static bool first = true;
    SetNextWindowSize(ImVec2(0.5f*io.DisplaySize.x,0.5f*io.DisplaySize.y), ImGuiSetCond_FirstUseEver);

    if (BeginDock("Tree",p_open)){
      ShowTree();
    }

    dlgdock[DLG_Tree] = GetCurrentDock();
    if (first){
      dlgdock[DLG_Tree]->setDetachedDockSize(0.5f*io.DisplaySize.x,0.5f*io.DisplaySize.y);
      dlgdock[DLG_Tree]->setSplitWeight(dlgsplitweight[DLG_Tree][0],dlgsplitweight[DLG_Tree][1]);
      first = false;
    }
    EndDock();
  }
}

// Strutural Info dialog //
static void DialogStructInfo(bool *p_open){
  ImGuiIO& io = GetIO();
  if (*p_open){
    static bool first = true;
    SetNextWindowSize(ImVec2(0.5f*io.DisplaySize.x,0.5f*io.DisplaySize.y), ImGuiSetCond_FirstUseEver);

    if (BeginDock("Structural Information",p_open)){
      // c2::set_scene_pointers(mainview->iscene);
      // Text("Current scene: %d\n",mainview->iscene);
      // Text("Is init?: %d\n",c2::isinit);
      // Text("Current file: %d\n",c2::idfile);
      // Text("File name: %s\n",c2::file);
      // Text("Scene name: %s\n",c2::name);
      // Text("Scene radius: %.2f\n",c2::scenerad);
      // Separator();
      // Text("Number of fields: %d\n",c2::nf);
      // Text("Reference field: %d\n",c2::iref);
      // Text("Field name: %s\n",c2::fieldname);
      // Separator();
      // Text("Number of atoms: %d\n",c2::nat);
      // Text("Number of molecules: %d\n",c2::nmol);
      // Text("Is molecular crystal: %d\n",c2::ismolecule);
      // Text("Cell vectors:\n");
      // Text("  %.10f %.10f %.10f\n",c2::avec[0][0],c2::avec[0][1],c2::avec[0][2]);
      // Text("  %.10f %.10f %.10f\n",c2::avec[1][0],c2::avec[1][1],c2::avec[1][2]);
      // Text("  %.10f %.10f %.10f\n",c2::avec[2][0],c2::avec[2][1],c2::avec[2][2]);
      // float a = std::sqrt(c2::avec[0][0]*c2::avec[0][0]+c2::avec[0][1]*c2::avec[0][1]+c2::avec[0][2]*c2::avec[0][2]);
      // float b = std::sqrt(c2::avec[1][0]*c2::avec[1][0]+c2::avec[1][1]*c2::avec[1][1]+c2::avec[1][2]*c2::avec[1][2]);
      // float c = std::sqrt(c2::avec[2][0]*c2::avec[2][0]+c2::avec[2][1]*c2::avec[2][1]+c2::avec[2][2]*c2::avec[2][2]);
      // Text("Cell lengths (bohr): %.10f %.10f %.10f\n",a,b,c);
      // Text("Cell lengths (ang): %.10f %.10f %.10f\n",a*0.529177,b*0.529177,c*0.529177);
    }
    dlgdock[DLG_StructInfo] = GetCurrentDock();
    if (first){
      dlgdock[DLG_StructInfo]->setDetachedDockSize(0.5f*io.DisplaySize.x,0.5f*io.DisplaySize.y);
      dlgdock[DLG_StructInfo]->setSplitWeight(dlgsplitweight[DLG_StructInfo][0],dlgsplitweight[DLG_StructInfo][1]);
      first = false;
    }
    EndDock();
  }
}
