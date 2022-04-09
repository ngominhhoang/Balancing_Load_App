from app import app
from flask import render_template
from flask import request, redirect, make_response, jsonify
from werkzeug.utils import secure_filename
from flask import send_file, send_from_directory, safe_join, abort
import os
import pickle
import pandas as pd
from datetime import datetime
import subprocess


app.config["FILE_UPLOADS"] = os.path.dirname(__file__) + "/static/file/uploads"

app.config["FILE_RESULTS"] = os.path.dirname(__file__) + "/static/file/uploads"

app.config["ALLOWED_CSV_EXTENSIONS"] = ["CSV"]


def save_data(dataset, loc_file, pre_loc_file_url = app.config["FILE_UPLOADS"]):
  loc_file = os.path.join(pre_loc_file_url,loc_file)
  with open(loc_file, 'wb') as filehandle:
      pickle.dump(dataset, filehandle)
def load_data(loc_file, pre_loc_file_url = app.config["FILE_UPLOADS"]):
  loc_file = os.path.join(pre_loc_file_url,loc_file)
  with open(loc_file, 'rb') as filehandle:
      dataset = pickle.load(filehandle)
  return dataset

def run_process(id,level="1",percentage="30"):
    process_name = os.path.join(app.config["FILE_RESULTS"],"Balancing_Load")
    id_name = os.path.join(app.config["FILE_RESULTS"],id)
    print(id_name)
    tmp=subprocess.Popen([process_name,percentage,level,id_name],stdout=subprocess.PIPE)
    out,err = tmp.communicate()
    out = out.decode("utf-8")
    str = out.split('\n')
    state = str[1].replace(' ','')
    phase_arr = [chr(ord(char[0]) + 17) for char in state]
    data_final_name = id + "_data_final"
    data = load_data(data_final_name)
    data['Pha mới'] = phase_arr
    str.pop(1)
    csv_final_path = os.path.join(app.config["FILE_RESULTS"],id + '_result.csv')
    data.to_csv(csv_final_path,index=False)
    csv_name = id + '_result.csv'
    print(str)
    return csv_name, str


def time_to_num(dt_time):
    return str(dt_time.year) + str(dt_time.month) + str(dt_time.day) + str(dt_time.hour) + str(dt_time.minute) + str(int(dt_time.second))

def allowed_file(filename):

    if not "." in filename:
        return False

    ext = filename.rsplit(".", 1)[1]

    if ext.upper() in app.config["ALLOWED_CSV_EXTENSIONS"]:
        return True
    else:
        return False

def convert_input(filename, id):
    filename = os.path.join(app.config["FILE_UPLOADS"], filename)
    data_raw = pd.read_csv(filename,sep=';')
    TS_list = []
    for i in range(1,10):
      col_name = 'Thông số '+str(i)
      if col_name in data_raw.columns:
        TS_list.append(col_name)
      else:
        break
    Total_list = TS_list + ['Tên khách hàng','Số cột (vị trí)','Pha']
    data_depend = data_raw[data_raw['Phụ thuộc - không phụ thuộc\n(0-1)'] == 0][Total_list]

    data_not_depend = data_raw[data_raw['Phụ thuộc - không phụ thuộc\n(0-1)'] == 1][Total_list]

    for ts in TS_list:
        data_not_depend[ts] = data_not_depend[ts].apply(lambda x: float(x.replace(',', '.')))

    for ts in TS_list:
        data_depend[ts] = data_depend[ts].apply(lambda x: float(x.replace(',', '.')))

    Ten_KH = data_depend.groupby(['Số cột (vị trí)'])['Tên khách hàng'].apply(','.join).reset_index()
    TS_1 = data_depend.groupby(['Số cột (vị trí)','Pha'])[TS_list].sum().reset_index()
    data_depend_final = pd.concat([Ten_KH['Tên khách hàng'],TS_1], axis=1)
    data_final = pd.concat([data_depend_final, data_not_depend], axis = 0)
    pandas_name = id + '_data_final'
    save_data(data_final,pandas_name)

    state = [str(ord(x) - 65) for x in data_final['Pha']]
    state_string = " "
    state_string = state_string.join(state)
    final_value = []
    for ts in TS_list:
      value = [str(x) for x in data_final[ts]]
      value_string = " "
      value_string = value_string.join(value)
      final_value.append(value_string)
    txt_name = id + '_input.txt'
    txt_file = os.path.join(app.config["FILE_UPLOADS"], txt_name)
    with open(txt_file, 'w') as writefile:
        writefile.write(str(len(data_final.index)))
        writefile.write('\n')
        writefile.write(state_string)
        writefile.write('\n')
        for value in final_value:
          writefile.write(value)
          writefile.write('\n')


@app.route("/about")
def about():
    return "All about Flask"
    
@app.route("/", methods=["GET", "POST"])
def upload_file():
    if request.method == "POST":
        if request.files:
            csv_file = request.files["uploadFile"]
            if csv_file.filename == "":
                print("No filename")
                return redirect(request.url)
            if allowed_file(csv_file.filename):
                try:
                    print(csv_file.filename)
                    filename = secure_filename(csv_file.filename)
                    print(filename)
                    level = request.form["level"]
                    percentage = request.form["percentage"]
                    file_path = os.path.join(app.config["FILE_UPLOADS"], filename)
                    print(file_path)
                    if os.path.exists(file_path):
                        os.remove(file_path)
                    csv_file.save(file_path)
                    date = datetime.now()
                    id = time_to_num(date)
                    convert_input(filename, id)
                    filenamecsv, messages = run_process(id,level,percentage)
                    print("\n".join(item for item in messages))
                    return jsonify({'status': 1, 'htmlresponse': render_template('public/response.html', filenamecsv=filenamecsv), 'msg': "\n".join('<p>'+item+'</p>' for item in messages)})
                except Exception as e:
                    print(e)
                    return jsonify({'status': 0, 'msg': 'Something went wrong, please check your uploading file again!!!'})
                """
                resp = make_response(data.to_csv())
                resp.headers["Content-Disposition"] = "attachment; filename=result.csv"
                resp.headers["Content-Type"] = "text/csv"
                print(resp.get_data())
                return resp
                """
                """result_name = filename
                try:
                    return send_from_directory(app.config["FILE_RESULTS"], result_name, as_attachment=True)
                except FileNotFoundError:
                    abort(404)
                """
                return redirect(request.url)
            else:
                print("That file extension is not allowed")
                return redirect(request.url)
                
    return render_template("public/upload_file_new.html")
