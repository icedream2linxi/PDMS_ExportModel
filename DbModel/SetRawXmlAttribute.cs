using NHibernate.Mapping.Attributes;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Reflection;
using System.Text;

namespace DbModel
{
	class SetRawXmlAttribute : RawXmlAttribute
	{
		public virtual string Id { get; set; }
		public virtual string Name { get; set; }
		public virtual Type Type { get; set; }
		public override string Content {
			get
			{
				return "<set name=\"" + Name + "\" order-by=\"id asc\">"
					+ "<key column=\"" + Id + "\" />"
					+ "<one-to-many class=\"" + HbmWriterHelper.GetNameWithAssembly(Type) + "\" />"
					+ "</set>";
			}
			set
			{
				
			}
		}
	}
}
