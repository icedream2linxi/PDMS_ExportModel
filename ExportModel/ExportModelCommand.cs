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
			try
			{
				ExportModel.ExportData export = new ExportModel.ExportData();
				export.Export();
			}
			catch (Exception ex)
			{
				String err = ex.StackTrace;
				Console.WriteLine(ex.Message);
				Console.WriteLine(err);
				if (ex.InnerException != null)
				{
					Console.WriteLine(ex.InnerException.Message);
					Console.WriteLine(ex.InnerException.StackTrace);
				}
			}
		}
	}
}
