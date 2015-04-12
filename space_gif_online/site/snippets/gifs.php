
  <div class="row">

    <?php foreach (page('gifs')->images()->flip() as $image) : ?>
      <div class="col-md-3 col-sm-4 col-xs-6">
        <a href="<?php echo $image->url() ?>">
          <img src="<?php echo $image->url() ?>" alt="" class="img-responsive impb">
        </a>
      </div>
    <?php endforeach ?>

  </div>

