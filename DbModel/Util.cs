using NHibernate;
using NHibernate.Cfg;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace DbModel
{
	public class Util : IDisposable
	{
		Configuration cf = null;
		private ISessionFactory sessionFactory = null;

		public ISessionFactory SessionFactory
		{
			get
			{
				return sessionFactory;
			}
		}

		~Util()
		{
			Dispose();
		}

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

				NHibernate.Mapping.Attributes.HbmSerializer.Default.Validate = true;
				using (FileStream fs = File.Create(@"f:\temp\nhibernate.xml"))
				{
					System.IO.MemoryStream stream1 = new System.IO.MemoryStream();
					NHibernate.Mapping.Attributes.HbmSerializer.Default.Serialize(
						stream1, System.Reflection.Assembly.GetExecutingAssembly());
					stream1.Position = 0;
					stream1.WriteTo(fs);
					stream1.Close();
				}
				System.IO.MemoryStream stream = new System.IO.MemoryStream();
				NHibernate.Mapping.Attributes.HbmSerializer.Default.Serialize(
					stream, System.Reflection.Assembly.GetExecutingAssembly());
				stream.Position = 0;
				cf.AddInputStream(stream);

				sessionFactory = cf.BuildSessionFactory();
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
				throw;
			}
		}

		public void Dispose()
		{
			if (sessionFactory != null)
			{
				sessionFactory.Dispose();
				sessionFactory = null;
			}
		}
	}
}
