using NHibernate.Mapping.Attributes;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Reflection;
using System.Text;

namespace DbModel
{
	class PointAttribute : RawXmlAttribute
	{
		private string type = HbmWriterHelper.GetNameWithAssembly(typeof(PointType));
		private string name;
		public virtual string Prefix { get; set; }
		public virtual string Name {
			get { return name; }
			set
			{
				name = value;
				if (Prefix == null && value != null)
					Prefix = value.ToLower();
			}
		}
		public override string Content {
			get
			{
				return "<property name=\"" + Name + "\">"
					+ "<column name=\"" + Prefix + "_x\"/>"
					+ "<column name=\"" + Prefix + "_y\"/>"
					+ "<column name=\"" + Prefix + "_z\"/>"
					+ "<type name=\"" + type + "\">"
					+ "<param name=\"prefix\">" + Prefix + "</param>"
					+ "</type>"
					+ "</property>";
			}
			set
			{
				
			}
		}
	}
}
