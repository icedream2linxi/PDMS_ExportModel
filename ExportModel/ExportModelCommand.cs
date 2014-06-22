using Aveva.ApplicationFramework.Presentation;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace Aveva.Presentation.AttributeBrowserAddin
{
	public class ExportModelCommand : Command
	{
		public ExportModelCommand()
		{
			this.Key = "Fulongtech.ExportModelCommand";
		}

		public override void Execute()
		{
			MessageBox.Show("ExportModel");
		}
	}
}
