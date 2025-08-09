from dash import Dash, dcc, html, dash_table, callback_context
import dash_leaflet as dl
from dash.dependencies import Input, Output, State
from dash.exceptions import PreventUpdate
import base64
import pandas as pd
import plotly.express as px
from AnimalShelter import AnimalShelter
from dotenv import load_dotenv
import os

cached_shelter = None

# Load .env credentials
load_dotenv()
MONGO_USERNAME = os.getenv("MONGO_USER")
MONGO_PASSWORD = os.getenv("MONGO_PASS")

# App
app = Dash(__name__, suppress_callback_exceptions=True)

# Layout
app.layout = html.Div([
    html.Center(html.B(html.H1("Animal Shelter Dashboard Enhanced - ADeMarco"))),
    html.Center(html.Img(
        src="data:image/png;base64,{}".format(base64.b64encode(open("GSL.png", "rb").read()).decode()),
        style={"height": "200px", "display": "block", "margin": "auto"}
    )),
    html.Hr(),

    html.Div([
        html.Label("Username:"), dcc.Input(id="input_user", type="text", placeholder="Enter Username"),
        html.Label("Password:"), dcc.Input(id="input_passwd", type="password", placeholder="Enter Password"),
        html.Button("Login", id="login-button", n_clicks=0),
    ]),
    html.Hr(),

    html.Div(id="data-table-container", children=[
    dash_table.DataTable(
        id="datatable-id",
        columns=[],
        data=[],
        page_size=10,
        row_selectable="single",
        selected_rows=[],
    )
]),

    html.Hr(),

    html.Div(id="filter-section", style={"display": "none"}, children=[
        html.Label("Select Rescue Type:"),
        dcc.RadioItems(
            id="rescue-type",
            options=[
                {"label": "Water Rescue", "value": "Water"},
                {"label": "Mountain or Wilderness Rescue", "value": "Mountain"},
                {"label": "Disaster or Individual Tracking", "value": "Disaster"},
                {"label": "Reset Filters", "value": "Reset"}
            ],
            value="Reset"
        ),
        html.Label("Preferred Dog Breeds:"),
        dcc.Dropdown(id="breed-filter", multi=True, value=[]),

        html.Label("Preferred Sex:"),
        dcc.RadioItems(
            id="sex-filter",
            options=[
                {"label": "Intact Male", "value": "Intact Male"},
                {"label": "Intact Female", "value": "Intact Female"}
            ],
            value="Intact Male"
        ),
        html.Label("Select Training Age Range (Weeks):"),
        dcc.RangeSlider(
            id="age-filter",
            min=20, max=300, step=1,
            marks={i: str(i) for i in range(20, 301, 50)},
            value=[20, 300]
        ),
        html.Br(),
        html.Button("Apply Filter", id="apply-filter-button", n_clicks=0, style={"margin-top": "10px"})
    ]),
    html.Br(), html.Hr(),

    html.Div(className="row", style={"display": "flex"}, children=[
        html.Div(id="graph-id", className="col s12 m6"),
        html.Div(id="map-id", className="col s12 m6")
    ])
])

# Main Callback for Authentication, Filtering, and Map
@app.callback(
    [Output("data-table-container", "children"),
     Output("filter-section", "style"),
     Output("graph-id", "children"),
     Output("map-id", "children")],
    [Input("login-button", "n_clicks"),
     Input("apply-filter-button", "n_clicks")],
    [State("input_user", "value"),
     State("input_passwd", "value"),
     State("breed-filter", "value"),
     State("sex-filter", "value"),
     State("age-filter", "value"),
     State("datatable-id", "selected_rows"),
     State("datatable-id", "derived_virtual_data")]
)
def authenticate_and_filter_data(login_clicks, filter_clicks,
                                 inputUser, inputPass,
                                 breed_filter, sex_filter, age_filter,
                                 selected_rows, derived_data):
    global cached_shelter
    ctx = callback_context
    if not ctx.triggered:
        raise PreventUpdate

    button_clicked = ctx.triggered[0]["prop_id"].split(".")[0]

    # Reset login
    if button_clicked == "login-button":
        cached_shelter = None
        user = inputUser.strip() if inputUser else MONGO_USERNAME
        passwd = inputPass.strip() if inputPass else MONGO_PASSWORD
        if user != (MONGO_USERNAME or "").strip() or passwd != (MONGO_PASSWORD or "").strip():
            return html.Div("Invalid credentials.", style={"color": "red"}), {"display": "none"}, "", ""

    try:
        if not cached_shelter:
            user = inputUser if inputUser else MONGO_USERNAME
            passwd = inputPass if inputPass else MONGO_PASSWORD
            cached_shelter = AnimalShelter(user, passwd)

        db = cached_shelter
        query = {}

        if button_clicked == "apply-filter-button":
            if breed_filter:
                query["breed"] = {"$in": breed_filter}
            if sex_filter:
                query["sex_upon_outcome"] = sex_filter
            if age_filter:
                query["age_upon_outcome_in_weeks"] = {"$gte": age_filter[0], "$lte": age_filter[1]}

        df = pd.DataFrame.from_records(db.read(query))

        if df.empty:
            return html.Div("No data found."), {"display": "block"}, "", ""

        if "_id" in df.columns:
            df.drop(columns=["_id"], inplace=True)

        pie_chart = dcc.Graph(figure=px.pie(df, names="breed", title="Breed Distribution"))

        # Handle map center based on selected row
        if selected_rows and derived_data:
            selected_index = selected_rows[0]
            selected_row = derived_data[selected_index]
            center_lat = selected_row["location_lat"]
            center_lon = selected_row["location_long"]
            markers = [
                dl.Marker(position=[center_lat, center_lon],
                          children=[
                              dl.Tooltip(selected_row["breed"]),
                              dl.Popup([
                                  html.H4(f"Name: {selected_row['name']}"),
                                  html.P(f"Breed: {selected_row['breed']}"),
                                  html.P(f"Sex: {selected_row['sex_upon_outcome']}"),
                                  html.P(f"Age (weeks): {selected_row['age_upon_outcome_in_weeks']}")
                              ])
                          ])
            ]
        else:
            center_lat = df["location_lat"].mean()
            center_lon = df["location_long"].mean()
            markers = [
                dl.Marker(position=[row["location_lat"], row["location_long"]],
                          children=[
                              dl.Tooltip(row["breed"]),
                              dl.Popup([
                                  html.H4(f"Name: {row['name']}"),
                                  html.P(f"Breed: {row['breed']}"),
                                  html.P(f"Sex: {row['sex_upon_outcome']}"),
                                  html.P(f"Age (weeks): {row['age_upon_outcome_in_weeks']}")
                              ])
                          ]) for _, row in df.iterrows()
            ]

        map_view = html.Div([
            dl.Map(
                style={"width": "1000px", "height": "500px"},
                center=[center_lat, center_lon],
                zoom=8,  # Try 8 for a wider zoom
                children=[dl.TileLayer()] + markers
            )
        ], id="map-wrapper", key=f"{center_lat}-{center_lon}")


        return (
            dash_table.DataTable(
                id="datatable-id",
                columns=[{"name": i, "id": i} for i in df.columns],
                data=df.to_dict("records"),
                page_size=10,
                sort_action="native",
                filter_action="native",
                row_selectable="single",
                selected_rows=[]
            ),
            {"display": "block"},
            pie_chart,
            map_view
        )

    except Exception as e:
        return html.Div("Error: " + str(e), style={"color": "red"}), {"display": "none"}, "", ""


# Breed filter options
@app.callback(
    Output("breed-filter", "options"),
    Input("rescue-type", "value")
)
def update_breed_options(rescue_type):
    breed_options = {
        "Water": ["Labrador Retriever Mix", "Chesapeake Bay Retriever", "Newfoundland"],
        "Mountain": ["German Shepherd", "Alaskan Malamute", "Old English Sheepdog", "Siberian Husky", "Rottweiler"],
        "Disaster": ["Doberman Pinscher", "German Shepherd", "Golden Retriever", "Bloodhound", "Rottweiler"]
    }
    return [{"label": breed, "value": breed} for breed in breed_options.get(rescue_type, [])]


# Run the app
if __name__ == "__main__":
    app.run(debug=True)
