using NHibernate.Mapping.Attributes;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DbModel
{
	class PointAttribute : RawXmlAttribute
	{
		private string type = HbmWriterHelper.GetNameWithAssembly(typeof(PointType));
		public virtual string Prefix { get; set; }
		public virtual string Name { get; set; }
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
