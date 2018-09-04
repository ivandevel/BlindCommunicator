package ar.com.lrusso.blindcommunicator;

import android.content.Context;
import android.database.*;
import android.os.*;
import android.provider.*;

import java.io.DataOutputStream;
import java.util.*;

public class ContactsListThread extends AsyncTask<Context, String, Boolean>
	{
	private Context context;

	@Override protected void onPreExecute()
		{
		super.onPreExecute();
		GlobalVars.contactListReady = false;
		}

	@Override protected Boolean doInBackground(Context... myContext)
		{
		context = myContext[0];

		writeFile("sizeofcontacts.cfg","0");

		GlobalVars.contactDataBase.clear();

		try
			{
			Cursor cursor = GlobalVars.context.getContentResolver().query(ContactsContract.CommonDataKinds.Phone.CONTENT_URI, null, null,null, null);
			while (cursor.moveToNext())
				{
				try
					{
					String name = cursor.getString(cursor.getColumnIndex(ContactsContract.CommonDataKinds.Phone.DISPLAY_NAME)).trim();
					String phoneNumber = cursor.getString(cursor.getColumnIndex(ContactsContract.CommonDataKinds.Phone.NUMBER)).trim();
					GlobalVars.contactDataBase.add(name.concat("|").concat(phoneNumber));
					}
					catch(NullPointerException e)
					{
					}
					catch(Exception e)
					{
					}
				}

		    // SORTING CONTACTS
			Collections.sort(GlobalVars.contactDataBase,new Comparator<String>(){public int compare(String s1, String s2){return s1.compareToIgnoreCase(s2);}});

		    // DELETING REPEATED VALUES
			try
				{
				for (int i=0;i<=GlobalVars.contactDataBase.size();i++)
					{
					try
						{
						String contact1 = GlobalVars.contactDataBase.get(i);
						String contact1Name = GlobalVars.getContactNameFromPhoneNumber(contact1);
						String contact1Phone = GlobalVars.contactsGetPhoneNumberFromListValue(contact1);
						
						for (int a=0;a<=GlobalVars.contactDataBase.size();a++)
							{
							try
								{
								String contact2 = GlobalVars.contactDataBase.get(a);
								String contact2Name = GlobalVars.getContactNameFromPhoneNumber(contact2);
								String contact2Phone = GlobalVars.contactsGetPhoneNumberFromListValue(contact2);
								
								if (contact1Name.equals(contact2Name) && contact1Phone.equals(contact2Phone) && i!=a)
									{
						        	GlobalVars.contactDataBase.remove(a);
									}
								}
								catch(Exception e)
								{
								}
							}
						}
						catch(Exception e)
						{
						}
					}
				}
				catch(Exception e)
				{
				}
			}
			catch(NullPointerException e)
			{
			}
			catch(Exception e)
			{
			}
		return false;
		}

	@Override protected void onPostExecute(Boolean pageloaded)
		{
		GlobalVars.contactListReady = true;

		writeFile("sizeofcontacts.cfg",String.valueOf(GlobalVars.contactDataBase.size()));
		}

	private void writeFile(String file, String text)
		{
        try
			{
            DataOutputStream out = new DataOutputStream(context.openFileOutput(file, Context.MODE_PRIVATE));
            out.writeUTF(text);
            out.close();
			}
			catch(Exception e)
			{
			}
		}
	
	private void removeDuplicates(List<String> list)
		{
	    final Set<String> encountered = new HashSet<String>();
	    for (Iterator<String> iter = list.iterator(); iter.hasNext(); )
	    	{
	        final String t = iter.next();
	        final boolean first = encountered.add(t);
	        if (!first)
	        	{
	            iter.remove();
	        	}
	    	}
		}	
	}