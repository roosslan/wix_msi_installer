#pragma once

#include "stdafx.h"
#include "ini_file.h"

namespace wix_installer {	
	static public ref class wix_builder	{
	public:
		/* static ILog^ _logger = LogManager::GetLogger("wix"); */
		static string config_file_path = nullptr;
		static ini_plain^ ini_file;
		static string set_up_log_config();
		static void add_directories(ManagedProject^ project, string rel_files_from_dir);
		static void msi_after_install(SetupEventArgs^ e);
		static void msi_before_install(SetupEventArgs^ e);
		static void ui_initialized(SetupEventArgs^ e);
		static void msi_load(SetupEventArgs^ e);
	};
}