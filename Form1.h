#pragma once
#pragma comment(linker, "/SUBSYSTEM:CONSOLE /ENTRY:WinMainCRTStartup")
#include "src\MQTriangleMesh.h"
#include "src\MQBasicUI.h"
#include "src\MQOpenGL.h"
#include "src\MQDotNETUtility.h"


namespace VC_viewer2010 {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for Form1
	/// </summary>
	public ref class Form1 : public System::Windows::Forms::Form
	{
	public:
		Form1(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
			this->TMesh = new MQTriangleMesh;
			this->BasicUI = new MQBasicUI;
			this->Texture = new MQTexture;
			this->ColR = 255;
			this->ColG = 0;
			this->ColB = 0;
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~Form1()
		{
			if (components)
			{
				delete components;
			}
		}

	private:
		MQTriangleMesh *TMesh;
		MQBasicUI		*BasicUI;
		MQTexture      *Texture;
		GLubyte		ColR, ColG, ColB;

	private: MQOpenGL::MQOpenGLControl^  mqOpenGLControl1;
	protected: 
	private: System::Windows::Forms::MenuStrip^  menuStrip1;
	private: System::Windows::Forms::ToolStripMenuItem^  fileToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  importobjToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  optionToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  loadTextureToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  enableTextureToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  enableModulateToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  enableMipmapToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  enableEnvironmentMapToolStripMenuItem;
	private: System::Windows::Forms::StatusStrip^  statusStrip1;
	private: System::Windows::Forms::ToolStripStatusLabel^  toolStripStatusLabel1;
	private: System::Windows::Forms::OpenFileDialog^  openFileDialog1;
	private: System::Windows::Forms::ToolStrip^  toolStrip1;
	private: System::Windows::Forms::ToolStripButton^  toolStripButton1;
	private: System::Windows::Forms::ToolStripButton^  toolStripButton2;

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			MQOpenGL::MQFont2DType^  mqFont2DType1 = (gcnew MQOpenGL::MQFont2DType());
			MQOpenGL::MQFont2DType^  mqFont2DType2 = (gcnew MQOpenGL::MQFont2DType());
			System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(Form1::typeid));
			this->mqOpenGLControl1 = (gcnew MQOpenGL::MQOpenGLControl());
			this->menuStrip1 = (gcnew System::Windows::Forms::MenuStrip());
			this->fileToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->importobjToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->optionToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->loadTextureToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->enableTextureToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->enableModulateToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->enableMipmapToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->enableEnvironmentMapToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->statusStrip1 = (gcnew System::Windows::Forms::StatusStrip());
			this->toolStripStatusLabel1 = (gcnew System::Windows::Forms::ToolStripStatusLabel());
			this->openFileDialog1 = (gcnew System::Windows::Forms::OpenFileDialog());
			this->toolStrip1 = (gcnew System::Windows::Forms::ToolStrip());
			this->toolStripButton1 = (gcnew System::Windows::Forms::ToolStripButton());
			this->toolStripButton2 = (gcnew System::Windows::Forms::ToolStripButton());
			this->menuStrip1->SuspendLayout();
			this->statusStrip1->SuspendLayout();
			this->toolStrip1->SuspendLayout();
			this->SuspendLayout();
			// 
			// mqOpenGLControl1
			// 
			this->mqOpenGLControl1->Dock = System::Windows::Forms::DockStyle::Fill;
			mqFont2DType1->Bold = MQOpenGL::FONTBOLD::DONTCARE;
			mqFont2DType1->Charset = MQOpenGL::FONTCHARSET::ANSI_SET;
			mqFont2DType1->Color = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(0)), static_cast<System::Int32>(static_cast<System::Byte>(0)), 
				static_cast<System::Int32>(static_cast<System::Byte>(0)));
			mqFont2DType1->Font = (gcnew System::Drawing::Font(L"Arial", 12, System::Drawing::FontStyle::Bold));
			mqFont2DType1->Pitch = MQOpenGL::FONTPITCH::DEFAULT;
			this->mqOpenGLControl1->Font2D_Type = mqFont2DType1;
			mqFont2DType2->Bold = MQOpenGL::FONTBOLD::DONTCARE;
			mqFont2DType2->Charset = MQOpenGL::FONTCHARSET::ANSI_SET;
			mqFont2DType2->Color = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(0)), static_cast<System::Int32>(static_cast<System::Byte>(0)), 
				static_cast<System::Int32>(static_cast<System::Byte>(0)));
			mqFont2DType2->Font = (gcnew System::Drawing::Font(L"Arial", 12, System::Drawing::FontStyle::Bold));
			mqFont2DType2->Pitch = MQOpenGL::FONTPITCH::DEFAULT;
			this->mqOpenGLControl1->Font3D_Type = mqFont2DType2;
			this->mqOpenGLControl1->Location = System::Drawing::Point(0, 24);
			this->mqOpenGLControl1->Margin = System::Windows::Forms::Padding(2, 2, 2, 2);
			this->mqOpenGLControl1->Name = L"mqOpenGLControl1";
			this->mqOpenGLControl1->Size = System::Drawing::Size(362, 340);
			this->mqOpenGLControl1->TabIndex = 0;
			this->mqOpenGLControl1->Load += gcnew System::EventHandler(this, &Form1::mqOpenGLControl1_Load);
			this->mqOpenGLControl1->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &Form1::mqOpenGLControl1_Paint);
			this->mqOpenGLControl1->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &Form1::mqOpenGLControl1_MouseDown);
			this->mqOpenGLControl1->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &Form1::mqOpenGLControl1_MouseMove);
			this->mqOpenGLControl1->MouseUp += gcnew System::Windows::Forms::MouseEventHandler(this, &Form1::mqOpenGLControl1_MouseUp);
			this->mqOpenGLControl1->Resize += gcnew System::EventHandler(this, &Form1::mqOpenGLControl1_Resize);
			// 
			// menuStrip1
			// 
			this->menuStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) {this->fileToolStripMenuItem, 
				this->optionToolStripMenuItem});
			this->menuStrip1->Location = System::Drawing::Point(0, 0);
			this->menuStrip1->Name = L"menuStrip1";
			this->menuStrip1->Padding = System::Windows::Forms::Padding(4, 2, 0, 2);
			this->menuStrip1->Size = System::Drawing::Size(362, 24);
			this->menuStrip1->TabIndex = 1;
			this->menuStrip1->Text = L"menuStrip1";
			// 
			// fileToolStripMenuItem
			// 
			this->fileToolStripMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) {this->importobjToolStripMenuItem});
			this->fileToolStripMenuItem->Name = L"fileToolStripMenuItem";
			this->fileToolStripMenuItem->Size = System::Drawing::Size(39, 20);
			this->fileToolStripMenuItem->Text = L"File";
			// 
			// importobjToolStripMenuItem
			// 
			this->importobjToolStripMenuItem->Name = L"importobjToolStripMenuItem";
			this->importobjToolStripMenuItem->Size = System::Drawing::Size(144, 22);
			this->importobjToolStripMenuItem->Text = L"Import *.obj";
			this->importobjToolStripMenuItem->Click += gcnew System::EventHandler(this, &Form1::importobjToolStripMenuItem_Click);
			// 
			// optionToolStripMenuItem
			// 
			this->optionToolStripMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(5) {this->loadTextureToolStripMenuItem, 
				this->enableTextureToolStripMenuItem, this->enableModulateToolStripMenuItem, this->enableMipmapToolStripMenuItem, this->enableEnvironmentMapToolStripMenuItem});
			this->optionToolStripMenuItem->Name = L"optionToolStripMenuItem";
			this->optionToolStripMenuItem->Size = System::Drawing::Size(60, 20);
			this->optionToolStripMenuItem->Text = L"Option";
			// 
			// loadTextureToolStripMenuItem
			// 
			this->loadTextureToolStripMenuItem->Name = L"loadTextureToolStripMenuItem";
			this->loadTextureToolStripMenuItem->Size = System::Drawing::Size(219, 22);
			this->loadTextureToolStripMenuItem->Text = L"Load Texture";
			this->loadTextureToolStripMenuItem->Click += gcnew System::EventHandler(this, &Form1::loadTextureToolStripMenuItem_Click);
			// 
			// enableTextureToolStripMenuItem
			// 
			this->enableTextureToolStripMenuItem->Name = L"enableTextureToolStripMenuItem";
			this->enableTextureToolStripMenuItem->Size = System::Drawing::Size(219, 22);
			this->enableTextureToolStripMenuItem->Text = L"Enable Texture";
			this->enableTextureToolStripMenuItem->Click += gcnew System::EventHandler(this, &Form1::enableTextureToolStripMenuItem_Click);
			// 
			// enableModulateToolStripMenuItem
			// 
			this->enableModulateToolStripMenuItem->Name = L"enableModulateToolStripMenuItem";
			this->enableModulateToolStripMenuItem->Size = System::Drawing::Size(219, 22);
			this->enableModulateToolStripMenuItem->Text = L"Enable Modulate";
			this->enableModulateToolStripMenuItem->Click += gcnew System::EventHandler(this, &Form1::enableModulateToolStripMenuItem_Click);
			// 
			// enableMipmapToolStripMenuItem
			// 
			this->enableMipmapToolStripMenuItem->Name = L"enableMipmapToolStripMenuItem";
			this->enableMipmapToolStripMenuItem->Size = System::Drawing::Size(219, 22);
			this->enableMipmapToolStripMenuItem->Text = L"Enable Mipmap";
			this->enableMipmapToolStripMenuItem->Click += gcnew System::EventHandler(this, &Form1::enableMipmapToolStripMenuItem_Click);
			// 
			// enableEnvironmentMapToolStripMenuItem
			// 
			this->enableEnvironmentMapToolStripMenuItem->Name = L"enableEnvironmentMapToolStripMenuItem";
			this->enableEnvironmentMapToolStripMenuItem->Size = System::Drawing::Size(219, 22);
			this->enableEnvironmentMapToolStripMenuItem->Text = L"Enable Environment Map";
			this->enableEnvironmentMapToolStripMenuItem->Click += gcnew System::EventHandler(this, &Form1::enableEnvironmentMapToolStripMenuItem_Click);
			// 
			// statusStrip1
			// 
			this->statusStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) {this->toolStripStatusLabel1});
			this->statusStrip1->Location = System::Drawing::Point(0, 342);
			this->statusStrip1->Name = L"statusStrip1";
			this->statusStrip1->Padding = System::Windows::Forms::Padding(1, 0, 10, 0);
			this->statusStrip1->Size = System::Drawing::Size(362, 22);
			this->statusStrip1->TabIndex = 2;
			this->statusStrip1->Text = L"statusStrip1";
			// 
			// toolStripStatusLabel1
			// 
			this->toolStripStatusLabel1->Name = L"toolStripStatusLabel1";
			this->toolStripStatusLabel1->Size = System::Drawing::Size(124, 17);
			this->toolStripStatusLabel1->Text = L"0 vertices, 0 triangles";
			// 
			// openFileDialog1
			// 
			this->openFileDialog1->InitialDirectory = L".";
			// 
			// toolStrip1
			// 
			this->toolStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) {this->toolStripButton1, 
				this->toolStripButton2});
			this->toolStrip1->Location = System::Drawing::Point(0, 24);
			this->toolStrip1->Name = L"toolStrip1";
			this->toolStrip1->Size = System::Drawing::Size(362, 25);
			this->toolStrip1->TabIndex = 3;
			this->toolStrip1->Text = L"toolStrip1";
			// 
			// toolStripButton1
			// 
			this->toolStripButton1->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"toolStripButton1.Image")));
			this->toolStripButton1->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->toolStripButton1->Name = L"toolStripButton1";
			this->toolStripButton1->Size = System::Drawing::Size(73, 22);
			this->toolStripButton1->Text = L"Draw2D";
			this->toolStripButton1->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &Form1::toolStripButton1_MouseDown);
			// 
			// toolStripButton2
			// 
			this->toolStripButton2->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"toolStripButton2.Image")));
			this->toolStripButton2->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->toolStripButton2->Name = L"toolStripButton2";
			this->toolStripButton2->Size = System::Drawing::Size(94, 22);
			this->toolStripButton2->Text = L"CheckHoles";
			this->toolStripButton2->Click += gcnew System::EventHandler(this, &Form1::toolStripButton2_Click);
			// 
			// Form1
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 12);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(362, 364);
			this->Controls->Add(this->toolStrip1);
			this->Controls->Add(this->statusStrip1);
			this->Controls->Add(this->mqOpenGLControl1);
			this->Controls->Add(this->menuStrip1);
			this->MainMenuStrip = this->menuStrip1;
			this->Margin = System::Windows::Forms::Padding(2);
			this->Name = L"Form1";
			this->Text = L"Form1";
			this->menuStrip1->ResumeLayout(false);
			this->menuStrip1->PerformLayout();
			this->statusStrip1->ResumeLayout(false);
			this->statusStrip1->PerformLayout();
			this->toolStrip1->ResumeLayout(false);
			this->toolStrip1->PerformLayout();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private: System::Void importobjToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
	{
		this->openFileDialog1->Filter = "obj files (*.obj)|*.obj";

		if(this->openFileDialog1->ShowDialog() == System::Windows::Forms::DialogResult::OK)
		{
			this->TMesh->ReadObjFile(MQString2string(this->openFileDialog1->FileName).c_str());
			this->toolStripStatusLabel1->Text = this->TMesh->VertexNum + " vertices, " + this->TMesh->TriangleNum + " triangles";
		}
	}

	private: System::Void mqOpenGLControl1_Load(System::Object^  sender, System::EventArgs^  e)
	{
		this->BasicUI->InitialSetup(this->mqOpenGLControl1->Width, this->mqOpenGLControl1->Height);
	}

	private: System::Void mqOpenGLControl1_MouseDown(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e)
	{
		if(e->Button == System::Windows::Forms::MouseButtons::Left)
		{
			this->BasicUI->Mode = MQBasicUI::TRACKBALL;
			this->BasicUI->MouseDown(e->X, e->Y);
		}
		else if(e->Button == System::Windows::Forms::MouseButtons::Middle)
		{
			this->BasicUI->Mode = MQBasicUI::TRANSLATE;
			this->BasicUI->MouseDown(e->X, e->Y);
		}
		else if(e->Button == System::Windows::Forms::MouseButtons::Right)
		{
			this->BasicUI->Mode = MQBasicUI::ZOOM;
			this->BasicUI->MouseDown(e->X, e->Y);
		}
	}

	private: System::Void mqOpenGLControl1_MouseMove(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e)
	{
		this->BasicUI->MouseMove(e->X, e->Y);
	}

	private: System::Void mqOpenGLControl1_MouseUp(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e)
	{
		this->BasicUI->MouseUp();
	}

	private: System::Void mqOpenGLControl1_Resize(System::Object^  sender, System::EventArgs^  e)
	{
		this->BasicUI->ViewportResize(this->mqOpenGLControl1->Width, this->mqOpenGLControl1->Height);
	}

	private: System::Void mqOpenGLControl1_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glPushMatrix();
		this->BasicUI->Paint();
		
		if(!this->toolStripButton1->Checked)	this->TMesh->Draw(this->ColR, this->ColG, this->ColB);
		else{
			this->TMesh->CalculateLaplacianToColor();
			this->TMesh->Draw2D();
		}
		
		glPopMatrix();
	}

	private: System::Void loadTextureToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
	{
		this->openFileDialog1->Filter = "bmp files (*.bmp)|*.bmp";

		if(this->openFileDialog1->ShowDialog() == System::Windows::Forms::DialogResult::OK)
		{
			this->mqOpenGLControl1->MakeMQOpenGLControlCurrent();

			this->Texture->LoadFromFile(this->openFileDialog1->FileName);

			gluBuild2DMipmaps(GL_TEXTURE_2D, this->Texture->Components, this->Texture->Width,
						   this->Texture->Height, this->Texture->Format, this->Texture->Type, this->Texture->Pixels);

			//compute simple texture coordinate
			for(int i = 1; i <= this->TMesh->VertexNum; i++)
			{
				this->TMesh->Vertex[i].S = this->TMesh->Vertex[i].X;
				this->TMesh->Vertex[i].T = this->TMesh->Vertex[i].Z;
			}

			this->enableTextureToolStripMenuItem->Checked = false;
			this->enableTextureToolStripMenuItem_Click(sender, e);

			this->mqOpenGLControl1->MakeMQOpenGLControlNotCurrent();
			this->mqOpenGLControl1->Refresh();
		}
	}

	private: System::Void enableTextureToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
	{
		this->enableTextureToolStripMenuItem->Checked = !(this->enableTextureToolStripMenuItem->Checked);

		this->mqOpenGLControl1->MakeMQOpenGLControlCurrent();

		if(this->enableTextureToolStripMenuItem->Checked)
		{
			this->ColR = 255;
			this->ColG = 255;
			this->ColB = 255;
			glEnable(GL_TEXTURE_2D);
		}
		else
		{
			glDisable(GL_TEXTURE_2D);
			this->ColR = 255;
			this->ColG = 0;
			this->ColB = 0;
		}

		this->mqOpenGLControl1->MakeMQOpenGLControlNotCurrent();
		this->mqOpenGLControl1->Refresh();
	}

	private: System::Void enableMipmapToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
	{
		this->enableMipmapToolStripMenuItem->Checked = !(this->enableMipmapToolStripMenuItem->Checked);

		this->mqOpenGLControl1->MakeMQOpenGLControlCurrent();

		if(this->enableMipmapToolStripMenuItem->Checked)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}

		this->mqOpenGLControl1->MakeMQOpenGLControlNotCurrent();
		this->mqOpenGLControl1->Refresh();
	}

	private: System::Void enableEnvironmentMapToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
	{
		this->enableEnvironmentMapToolStripMenuItem->Checked = !(this->enableEnvironmentMapToolStripMenuItem->Checked);

		this->mqOpenGLControl1->MakeMQOpenGLControlCurrent();

		if(this->enableEnvironmentMapToolStripMenuItem->Checked)
		{
			glTexGenf(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
			glTexGenf(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
			glEnable(GL_TEXTURE_GEN_S);
			glEnable(GL_TEXTURE_GEN_T);
		}
		else
		{
			glDisable(GL_TEXTURE_GEN_S);
			glDisable(GL_TEXTURE_GEN_T);
		}

		this->mqOpenGLControl1->MakeMQOpenGLControlNotCurrent();
		this->mqOpenGLControl1->Refresh();
	}

	private: System::Void enableModulateToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
	{
		this->enableModulateToolStripMenuItem->Checked = !(this->enableModulateToolStripMenuItem->Checked);

		this->mqOpenGLControl1->MakeMQOpenGLControlCurrent();

		if(this->enableModulateToolStripMenuItem->Checked)
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		else
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

		this->mqOpenGLControl1->MakeMQOpenGLControlNotCurrent();
		this->mqOpenGLControl1->Refresh();
	}
private: System::Void toolStripButton1_MouseDown(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {
			 this->toolStripButton1->Checked = !this->toolStripButton1->Checked;
		 }
private: System::Void toolStripButton2_Click(System::Object^  sender, System::EventArgs^  e) {

			 this->TMesh->FindHole();
		 }
};
}
