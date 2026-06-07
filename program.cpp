#include "stdafx.h"
#include "consts.h"
#include "program.h"
#include "ini_file.h"

namespace wix_installer {

    ref class CMain {

/* ===== f-n main() ====================================================================================================== */
        static int main(array<String^>^ args) {

            wix_builder::set_up_log_config();
            auto addin_version = gcnew String(addin_auto_version);

            /* AppDomain::CurrentDomain->AssemblyResolve += gcnew ResolveEventHandler(FindAssembly); */

            string app_data_path = Environment::GetFolderPath(Environment::SpecialFolder::ApplicationData);

            auto rel_files_from_dir = gcnew String(relativ_files_from_dir);       /* The path where files from (relative) */

            auto project = gcnew ManagedProject("net8project" + addin_version,
                gcnew Dir("%AppDataFolder%\\roosslan\\net8project",               /* INSTALLDIR */
                    gcnew WixSharp::Files(rel_files_from_dir + "\\*.*")));

            wix_builder::add_directories(project, rel_files_from_dir);

            /*
            Console::WriteLine("Listing project files...");
            for each (WixSharp::File^ f in project->AllFiles)
            {
                Console::WriteLine(f->Name);
            }
            project->ResolveWildCards()->FindFile(f->Name->EndsWith("ExportToApplication.exe"))->First()
                    .Shortcuts = new[] { new FileShortcut("ExportTo", desktopDirectoryPath) };
            */

            project->Scope = WixSharp::InstallScope::perUser;

            project->OutFileName = "net8project_setup_v" + addin_version;
            project->ControlPanelInfo->Manufacturer = "roosslan";
            project->Language = "ru-RU";

            project->GUID = Guid("ca11ab1e-DEAF-DADD-FACE-BA11ADE4CAFE");                                  
            project->UpgradeCode = Guid("cabba1a3-DEAD-BABE-B00B-E5ca1ade10ad");

            /* Добав.новой.ф-ции.Изменение/доработка ф-ции.Фикс.бага/ребилд/год */
            project->Version = gcnew Version(addin_version);

            auto pass_param = gcnew WixSharp::PublicProperty("HTTP", "www.roosslan.com");
            project->Properties = gcnew ::cli::array<WixSharp::Property^>(1);
            project->Properties->SetValue(pass_param, 0);
            
            /* MajorUpgrade^ majorUpgrade = gcnew MajorUpgrade();
            project->MajorUpgrade = gcnew MajorUpgrade();
            project->MajorUpgrade->AllowSameVersionUpgrades = true;
            project->MajorUpgrade->Schedule = UpgradeSchedule::afterInstallInitialize;
            project->MajorUpgrade->DowngradeErrorMessage = "You have newer version of addin.";
            project->MajorUpgrade = majorUpgrade;    */

            project->MajorUpgradeStrategy = MajorUpgradeStrategy::Default;
            project->MajorUpgradeStrategy->RemoveExistingProductAfter = Step::InstallInitialize;


            project->ManagedUI = gcnew ManagedUI();
            /* project->ManagedUI->InstallDialogs->Add(Dialogs::Welcome); */
            project->ManagedUI->InstallDialogs->Add(Dialogs::Licence);
            project->ManagedUI->InstallDialogs->Add(Dialogs::Progress);
            project->ManagedUI->InstallDialogs->Add(Dialogs::Exit);

            project->ManagedUI->ModifyDialogs->Add(Dialogs::MaintenanceType);
            project->ManagedUI->ModifyDialogs->Add(Dialogs::Features);
            project->ManagedUI->ModifyDialogs->Add(Dialogs::Progress);
            project->ManagedUI->ModifyDialogs->Add(Dialogs::Exit);
            project->ManagedUI->ModifyDialogs->Add(Dialogs::InstallScope);

            project->UIInitialized += gcnew ManagedProject::SetupEventHandler(&wix_builder::ui_initialized);

            project->BeforeInstall += gcnew ManagedProject::SetupEventHandler(&wix_builder::msi_before_install);

            project->Load += gcnew ManagedProject::SetupEventHandler(&wix_builder::msi_load);

            project->AfterInstall += gcnew ManagedProject::SetupEventHandler(&wix_builder::msi_after_install);

            project->BannerImage = "top_banner.gif";
            project->BackgroundImage = "background.gif";
            project->LicenceFile = "license.rtf";

            project->BuildMsi("wix\\" + project->OutFileName + ".msi");

            return 0;
        }
    }; /* <-- we need semicolon here because main() of CLR++ should be wrapped in a class like CMain */

    void wix_builder::add_directories(ManagedProject^ project, string rel_files_from_dir) {
        WixSharp::CommonTasks::Tasks::AddDir(project, gcnew Dir("%AppDataFolder%\\Autodesk\\Revit\\Addins\\2026", gcnew WixSharp::File(rel_files_from_dir + "\\Files\\bi_loader.addin")));
        WixSharp::CommonTasks::Tasks::AddDir(project, gcnew Dir("%AppDataFolder%\\Autodesk\\Revit\\Addins\\2026", gcnew WixSharp::File(rel_files_from_dir + "\\Files\\ext_plugin.addin")));

        
        /* app.config служит для указания пути к логу */
		WixSharp::CommonTasks::Tasks::AddDir(project, gcnew Dir("%AppDataFolder%\\net8project", gcnew WixSharp::File(rel_files_from_dir + "\\Files\\app.config")));
        WixSharp::CommonTasks::Tasks::AddDir(project, gcnew Dir("%AppDataFolder%\\net8project", gcnew WixSharp::File(rel_files_from_dir + "\\Files\\ifcxeprt.inf")));
    };

    string wix_builder::set_up_log_config() {
        string app_data_directory = Environment::GetFolderPath(Environment::SpecialFolder::ApplicationData);
        //IniFile^ ini_File = nullptr;
        Directory::CreateDirectory(app_data_directory + "\\net8project");

        try {
            ini_file = gcnew ini_plain(app_data_directory + "\\net8project\\ifcexprt.inf");
        }
        catch (const std::exception&){
            System::IO::File::AppendAllText(app_data_directory + "\\net8project\\net8project.wix.log", DateTime::Now.ToString("dd.MM.yyyy hh:mm tt") + "wix: The inf-file not found!\n");
        }
        config_file_path = ini_file->read_string("wix_net8project", "log4net_config");

        if (config_file_path == ""){
            System::IO::File::AppendAllText(app_data_directory + "\\net8project\\net8project.log", DateTime::Now.ToString("dd.MM.yyyy hh:mm tt") + "'[wix] log4net_appdata_config= ' key not found!\n");
                /* throw gcnew FileNotFoundException("'[wix] log4net_appdata_config= ' key not found!"); */
        }
        config_file_path = app_data_directory + "\\" + config_file_path;

        return config_file_path;
    }

    void wix_builder::ui_initialized(SetupEventArgs^ e) {
/*
		if (e->IsInstalling)
		  if (e->Session["SETUP_GUID"] != "cabba1a2-dead-babe-b00b-ca11ab1e10ad"){
		      System::Windows::Forms::MessageBox::Show(e->Session["SETUP_GUID"]);
		      e->ManagedUI->Shell->CustomErrorDescription = "Для установки плагина, пожалуйста, запустите setup.exe";
		      e->ManagedUI->Shell->ErrorDetected = true;
		      e->Result = ActionResult::UserExit;
		  }
*/
        Version^ installed_version = WixSharp::Extensions::LookupInstalledVersion(e->Session);
        Version^ this_version = WixSharp::Extensions::QueryProductVersion(e->Session);

        if (installed_version != nullptr) {
            if (this_version <= installed_version) {
                e->ManagedUI->Shell->ErrorDetected = true;
                e->ManagedUI->Shell->CustomErrorDescription = "You have newer version of addin.\nVersion: " + installed_version;
                e->Result = ActionResult::UserExit;
            }
            else {
                /* e->ManagedUI->Shell->Dialogs->Remove(Dialogs::Welcome); */
                e->ManagedUI->Shell->Dialogs->Remove(Dialogs::Licence);
                e->ManagedUI->Shell->Dialogs->Insert(0, Dialogs::Features);
            }
        }
    }

    void wix_builder::msi_load(SetupEventArgs^ e) {
        if (e->IsUninstalling) {
            array<Process^>^ arr_hprocesses = Process::GetProcessesByName("bgHelper");
            if (arr_hprocesses->Length > 0)
            {
                try {
                    for each (Process ^ process in arr_hprocesses) {
                        process->Kill();
                        process->WaitForExit();
                    }
                }
                catch (...) {

                }
            }
            array<Process^>^ arr_dprocesses = Process::GetProcessesByName("dbchecker");
            if (arr_dprocesses->Length > 0) {
                try {
                    for each (Process ^ process in arr_dprocesses) {
                        process->Kill();
                        process->WaitForExit();
                    }
                }
                catch (...) {}
            }
        }
    }

    void wix_builder::msi_before_install(SetupEventArgs^ e) {
        string path_to_file = System::IO::Path::Combine(e->InstallDir + "bgHelper.exe");
        string path_to_db_checker = System::IO::Path::Combine(e->InstallDir + "dbchecker.exe");

        for each (Process ^ process in Process::GetProcessesByName("bgHelper")) {
            try {
                process->Kill();
            }
            catch (...) {
                /* У некоторых пользователей System::Diagnostics::Process->Kill() кидает System::ComponentModel::Win32Exception: Отказано в доступе */
            }
        }
        for each (Process ^ process in Process::GetProcessesByName("dbchecker")) {
            try {
                process->Kill();
            }
            catch (...) {
                /* У некоторых пользователей Process->Kill() выкидывает Win32Exception: Отказано в доступе */
            }
        }

        if (e->IsInstalling) {
            try {
                Microsoft::Win32::RegistryKey^ key = Microsoft::Win32::Registry::CurrentUser->OpenSubKey("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", true);
                key->SetValue("ifc_exporter", path_to_file);
                key->SetValue("db_checker", path_to_db_checker);
            }
            catch (...) {

            }
        }
    }

    void wix_builder::msi_after_install(SetupEventArgs^ e) {
        string path_to_helper = System::IO::Path::Combine(e->InstallDir + "bgHelper.exe");
        string path_to_db_checker = System::IO::Path::Combine(e->InstallDir + "dbchecker.exe");

        if (e->IsInstalling) {
            try  {
                Process::Start(path_to_helper);
/*              Process::Start(pathToWrksctrl);         */
                Process::Start(path_to_db_checker);
            }
            catch (...){

            }
        }

        if (e->IsUninstalling) {
            try {            
                Microsoft::Win32::RegistryKey^ key = Microsoft::Win32::Registry::CurrentUser->OpenSubKey("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", true);
                key->DeleteValue("ifc_exporter", false);
                key->DeleteValue("db_checker", false);
            }
            catch (...){

            }
        }
    }
}

