using NHibernate;
using NHibernate.Cfg;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DbModel
{
	public class Util
	{
		Configuration cf = null;
		public void init(String dbPath)
		{
			IDictionary<String, String> settings = new Dictionary<String, String>();
			settings.Add("dialect", "NHibernate.Dialect.SQLiteDialect");
			settings.Add("connection.provider", "NHibernate.Connection.DriverConnectionProvider");
			settings.Add("connection.driver_class", "NHibernate.Driver.SQLite20Driver");
			settings.Add("connection.connection_string", "Data Source=" + dbPath + ";Version=3");
			settings.Add("hbm2ddl.auto", "create");

			try
			{
				cf = new Configuration();
				cf.Properties = settings;

				System.IO.MemoryStream stream = new System.IO.MemoryStream();
				NHibernate.Mapping.Attributes.HbmSerializer.Default.Validate = true;
				NHibernate.Mapping.Attributes.HbmSerializer.Default.Serialize(
					stream, System.Reflection.Assembly.GetExecutingAssembly());
				stream.Position = 0;
				cf.AddInputStream(stream);

				using(ISessionFactory sf = cf.BuildSessionFactory())
				using(ISession session = sf.OpenSession())
				using(ITransaction tx = session.BeginTransaction())
				{
					Box b = new Box();
					Point org = new Point(1, 2, 3);
					b.Org = org;
					session.Save(b);
					tx.Commit();
				}
			}
			catch (System.Exception ex)
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
