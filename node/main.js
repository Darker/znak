import express from "express"
import path from "path"
import bodyParser from "body-parser";

const HTTP_PORT = 1*process.env.PORT || 16661;

const app = express();

app.use(bodyParser.json());
app.use("/", express.static(path.resolve("./web")));

app.listen(HTTP_PORT, ()=>{
    console.log("Listening on port "+HTTP_PORT);
});