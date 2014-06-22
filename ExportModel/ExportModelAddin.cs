using Aveva.ApplicationFramework;
using Aveva.ApplicationFramework.Presentation;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Aveva.Presentation.AttributeBrowserAddin
{
	class ExportModelAddin : IAddin
	{
		public string Description
		{
			get { return "Export PDMS model"; }
		}

		public string Name
		{
			get { return "ExportModel"; }
		}

		public void Start(ServiceManager serviceManager)
		{
			CommandManager commandManager = (CommandManager)serviceManager.GetService(typeof(CommandManager));
			ExportModelCommand showCommand = new ExportModelCommand();
			commandManager.Commands.Add(showCommand);

			CommandBarManager commandBarManager = (CommandBarManager)serviceManager.GetService(typeof(CommandBarManager));
			commandBarManager.AddUICustomizationFile("ExportModel.uic", "ExportModel");
		}

		public void Stop()
		{
		}
	}
}
