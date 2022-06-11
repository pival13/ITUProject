#Need pandas 1.3 for this one
import pandas as pd

FILE_NAME = "guzergah_all"
pd.read_xml(f"xml_files/{FILE_NAME}.xml").to_csv(f"csv_files/{FILE_NAME}.csv")