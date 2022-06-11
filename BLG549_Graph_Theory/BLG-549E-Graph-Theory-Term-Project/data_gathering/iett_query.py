#%%
import json

import pandas as pd
import zeep
from zeep.cache import SqliteCache
from zeep.transports import Transport
from lxml import etree

#%%
transport = Transport(cache=SqliteCache())
# %% WSDL URL's
iett_hat_durak_wsdl = "https://api.ibb.gov.tr/iett/UlasimAnaVeri/HatDurakGuzergah.asmx?wsdl"
ibb_crm_wsdl = "https://api.ibb.gov.tr/iett/ibb/ibb.asmx?wsdl"

#%%
client = zeep.Client(wsdl=iett_hat_durak_wsdl, transport=transport)
print(client.service._operations.keys())

# %%
server_resp = client.service.GetDurak_json("")
pd.read_json(server_resp).to_csv("csv_files/durak_all.csv")

#%%
client = zeep.Client(wsdl=ibb_crm_wsdl, transport=transport)
print(client.service._operations.keys())

# %%
server_resp = client.service.DurakDetay_GYY("")
et = etree.ElementTree(server_resp)
#Need to delete closing tags until NewDataset in the EOF for pandas xml parsing
et.write("xml_files/guzergah_all.xml", pretty_print=True, encoding="utf-8")
#%%
server_resp = client.service.HatServisi_GYY("")
et = etree.ElementTree(server_resp)
#Need to delete closing tags until NewDataset in the EOF for pandas xml parsing
et.write("xml_files/hat_all.xml", pretty_print=True, encoding="utf-8")
# %%
