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

			/* Все директории и файлы из rel_files_from_dir будут добавлены в .msi */
            auto rel_files_from_dir = gcnew String(relativ_files_from_dir);

            auto project = gcnew ManagedProject("net8project" + addin_version,
                gcnew Dir("%AppDataFolder%\\roosslan\\net8project",               /* INSTALLDIR */
                    gcnew WixSharp::Files(rel_files_from_dir + "\\*.*",
                                          gcnew Predicate<string>(&wix_builder::is_payload))));

            wix_builder::add_directories(project, rel_files_from_dir);

            /*
            Console::WriteLine("Listing project files...");
            for each (WixSharp::File^ f in project->AllFiles)
            {
                Console::WriteLine(f->Name);
            }
            project->ResolveWildCards()->FindFile(f->Name->EndsWith("ifc_exporter.exe"))->First()
                    .Shortcuts = new[] { new FileShortcut("IFC Exporter", desktopDirectoryPath) };
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

        /* Надстройка ifc_exporter: Build\Addins\<год>\ifc_exporter.addin и Build\Addins\<год>\ifc_exporter\
         * (раскладку готовит CI репозитория ifc_exporter_revit_addin) */
        for each (string year in gcnew array<string>{ "2023", "2026" }) {
            string addin_src = rel_files_from_dir + "\\Addins\\" + year;
            if (!System::IO::Directory::Exists(addin_src))
                continue;   /* при сборке без надстройки MSI собирается как раньше */
            WixSharp::CommonTasks::Tasks::AddDir(project, gcnew Dir("%AppDataFolder%\\Autodesk\\Revit\\Addins\\" + year,
                gcnew WixSharp::File(addin_src + "\\ifc_exporter.addin"),
                gcnew Dir("ifc_exporter", gcnew WixSharp::Files(addin_src + "\\ifc_exporter\\*.*"))));
        }

        /* Автозагрузка через таблицу Registry MSI: при удалении записи удаляются автоматически */
        string run_key = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";
        WixSharp::CommonTasks::Tasks::AddRegValue(project, gcnew RegValue(RegistryHive::CurrentUser, run_key, "ifc_exporter", "\"[INSTALLDIR]bgHelper.exe\""));
        WixSharp::CommonTasks::Tasks::AddRegValue(project, gcnew RegValue(RegistryHive::CurrentUser, run_key, "db_checker", "\"[INSTALLDIR]dbchecker.exe\""));
    };

    /* Фильтр файлов для .msi: отладочные файлы не включаются, содержимое Files\ и Addins\ раскладывается в add_directories */
    bool wix_builder::is_payload(string path) {
        string ext = Path::GetExtension(path)->ToLowerInvariant();
        if (ext == ".pdb" || ext == ".ilk" || ext == ".log")
            return false;
        if (path->IndexOf("\\Files\\", StringComparison::OrdinalIgnoreCase) >= 0)
            return false;
        if (path->IndexOf("\\Addins\\", StringComparison::OrdinalIgnoreCase) >= 0)
            return false;
        return true;
    }

    /* Завершение вспомогательного процесса только в сеансе текущего пользователя */
    void wix_builder::stop_processes(SetupEventArgs^ e, string name) {
        int my_session = Process::GetCurrentProcess()->SessionId;
        for each (Process^ process in Process::GetProcessesByName(name)) {
            try {
                if (process->SessionId != my_session)
                    continue;   /* процессы других сеансов не трогаем: отсюда и было "Отказано в доступе" */
                process->Kill();
                if (!process->WaitForExit(5000))
                    e->Session->Log("wix: " + name + " (PID " + process->Id + ") не завершился за 5 с");
            }
            catch (InvalidOperationException^) {
                /* процесс уже завершился */
            }
            catch (System::ComponentModel::Win32Exception^ ex) {
                e->Session->Log("wix: не удалось завершить " + name + ": " + ex->Message);
            }
            finally {
                delete process;
            }
        }
    }

    string wix_builder::set_up_log_config() {
        /* Внимание: вызывается из main(), т.е. на машине сборки, а не у пользователя */
        string app_data_directory = Path::Combine(Environment::GetFolderPath(Environment::SpecialFolder::ApplicationData), "net8project");
        string log_path = Path::Combine(app_data_directory, "net8project.wix.log");
        string inf_path = Path::Combine(app_data_directory, "ifcxeprt.inf");
        Directory::CreateDirectory(app_data_directory);

        try {
            if (!System::IO::File::Exists(inf_path)) {
                System::IO::File::AppendAllText(log_path, DateTime::Now.ToString("dd.MM.yyyy HH:mm:ss") + " wix: " + inf_path + " не найден" + Environment::NewLine);
                return nullptr;
            }
            ini_file = gcnew ini_plain(inf_path);
            config_file_path = ini_file->read_string("wix_net8project", "log4net_config");

            if (String::IsNullOrEmpty(config_file_path)) {
                System::IO::File::AppendAllText(log_path, DateTime::Now.ToString("dd.MM.yyyy HH:mm:ss") + " wix: ключ [wix_net8project] log4net_config не найден" + Environment::NewLine);
                return nullptr;
            }
            config_file_path = Path::Combine(Environment::GetFolderPath(Environment::SpecialFolder::ApplicationData), config_file_path);
        }
        catch (Exception^ ex) {
            /* исключения .NET не ловятся через catch (const std::exception&) */
            System::IO::File::AppendAllText(log_path, DateTime::Now.ToString("dd.MM.yyyy HH:mm:ss") + " wix: " + ex->Message + Environment::NewLine);
            config_file_path = nullptr;
        }

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
            stop_processes(e, "bgHelper");
            stop_processes(e, "dbchecker");
        }
    }

    void wix_builder::msi_before_install(SetupEventArgs^ e) {
        /* Записи автозагрузки создаются в add_directories через RegValue */
        stop_processes(e, "bgHelper");
        stop_processes(e, "dbchecker");
    }

    void wix_builder::msi_after_install(SetupEventArgs^ e) {
        /* Записи автозагрузки удаляет сам MSI при деинсталляции */
        if (!e->IsInstalling)
            return;

        for each (string exe in gcnew array<string>{ "bgHelper.exe", "dbchecker.exe" }) {
            string path = Path::Combine(e->InstallDir, exe);
            try {
                Process::Start(path);
            }
            catch (Exception^ ex) {
                e->Session->Log("wix: не удалось запустить " + path + ": " + ex->Message);
            }
        }
    }
}

